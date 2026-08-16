//
// Created by white on 26. 4. 21..
//

#include "BoxCollider.h"
#include "CapsuleCollider.h"
#include "Collider.h"
#include "CollisionSolver.h"
#include "MeshCollider.h"
#include "SphereCollider.h"
#include "../Transform.h"
#include "../GameObjectArgument.h"
#include "../rigidBody/Rigidbody.h"
constexpr float EPSILON = 1e-4f;
CollisionFunc CollisionSolver::dispatchTable[(int)ColliderType::MaxCount][(int)ColliderType::MaxCount] = {};

namespace {
    // 로컬 대각 역관성을 월드 공간 벡터에 적용: I⁻¹_world·v = R·(I⁻¹_local·(Rᵀ·v))
    Vector3 ApplyInvInertia(const Rigidbody* rb, const Quaternion& rot, const Vector3& v) {
        Vector3 local = rot.Conjugate() * v;
        local = Vector3(local.x * rb->inverseInertiaLocal.x,
                        local.y * rb->inverseInertiaLocal.y,
                        local.z * rb->inverseInertiaLocal.z);
        return rot * local;
    }
}
float CollisionSolver::CombineMaterial(float a, float b,CombineMode modeA, CombineMode modeB) {
    // 두 객체의 CombineMode 중 우선순위가 높은 쪽을 따름
    // 우선순위: Maximum > Multiply > Minimum > Average
    CombineMode mode = (static_cast<int>(modeA) > static_cast<int>(modeB)) ? modeA : modeB;

    switch (mode) {
        case CombineMode::Average:  return (a + b) * 0.5f;
        case CombineMode::Minimum:  return (std::min)(a, b);
        case CombineMode::Multiply: return a * b;
        case CombineMode::Maximum:  return (std::max)(a, b);
        default:                    return (a + b) * 0.5f;
    }
}
// region Init & Use
/*
 *    S  B  C  M
 * S F  F  F  F
 * B     F  F  F
 * C         F  F
 * M            F
 */
void CollisionSolver::Initialize() {
    dispatchTable[0][0] = &SphereVsSphere;
    dispatchTable[0][1] = &SphereVsBox;
    dispatchTable[0][2] = &SphereVsCapsule;
    dispatchTable[0][3] = &SphereVsMesh;

    dispatchTable[1][1] = &BoxVsBox;
    dispatchTable[1][2] = &BoxVsCapsule;
    dispatchTable[1][3] = &BoxVsMesh;

    dispatchTable[2][2] = &CapsuleVsCapsule;
    dispatchTable[2][3] = &CapsuleVsMesh;

    dispatchTable[3][3] = &MeshVsMesh;
}

bool CollisionSolver::CheckCollision(Collider *a, Collider *b, Contact &outContact) {
    int typeA = (int)a->GetShapeType();
    int typeB = (int)b->GetShapeType();

    // 항상 타입 번호가 낮은 쪽을 a로, 높은 쪽을 b로 맞춤 (상삼각 행렬 규칙)
    if (typeA > typeB) {
        bool collided = dispatchTable[typeB][typeA](b, a, outContact);
        if (collided) {
            // a와 b가 뒤집혀서 검사되었으므로, 방향(Normal)과 주체(colA, colB)를 원상 복구
            outContact.normal = outContact.normal * -1.0f;
            std::swap(outContact.colA, outContact.colB);
            std::swap(outContact.rbA, outContact.rbB);
        }
        return collided;
    }

    // 순서가 맞다면 그대로 호출
    return dispatchTable[typeA][typeB](a, b, outContact);
}
float CollisionSolver::ResolveCollision(const Contact &contact) {
    Rigidbody* rbA = contact.rbA;
    Rigidbody* rbB = contact.rbB;

    // =========================================================
    // 예외 처리의 핵심: "Rigidbody가 없으면 무한 질량(Static)으로 취급한다"
    // =========================================================
    // rb가 nullptr이거나 isKinematic이면 역질량(invMass)을 0으로 만듭니다.
    // 역질량이 0이라는 것은 질량이 무한대(∞)라는 뜻이므로,
    // 어떤 힘을 받아도 절대 밀리지 않는 "완벽한 벽"으로 작동하게 됩니다.
    float invMassA = (rbA && !rbA->isKinematic) ? (1.0f / rbA->mass) : 0.0f;
    float invMassB = (rbB && !rbB->isKinematic) ? (1.0f / rbB->mass) : 0.0f;

    float totalInvMass = invMassA + invMassB;
    if (totalInvMass <= 0.0f) return 0.0f;

    const ColliderMaterial& matA = contact.colA->material;
    const ColliderMaterial& matB = contact.colB->material;

    // 반발계수 조합
    float restitution = CombineMaterial(
        matA.bounciness, matB.bounciness,
        matA.bounceCombine, matB.bounceCombine
    );
    constexpr float BOUNCE_THRESHOLD = 2.0f;

    // 마찰계수 조합 (동적)
    float friction = CombineMaterial(
        matA.dynamicFriction, matB.dynamicFriction,
        matA.frictionCombine, matB.frictionCombine
    );
    float staticFrictionC = CombineMaterial(
        matA.staticFriction, matB.staticFriction,
        matA.frictionCombine, matB.frictionCombine
    );

    // 3. 위치 보정 (Position Resolution) - 파고든 만큼 질량비로 밀어냄
    const float SLOP = 0.01f;
    // 짓눌림처럼 여러 접촉이 동시에 겹치면 penetration이 순간적으로 커질 수 있는데, CCD(스윕 테스트)가
    // 없는 이 엔진에서 한 번에 너무 크게 밀면 그 이동거리가 바닥 같은 얇은 지오메트리를 뚫고 지나가버림.
    // 한 번의 보정량에 상한을 걸어서(Box2D의 max linear correction과 동일한 이유) 큰 침투는 여러
    // 패스/틱에 걸쳐 나눠서 밀려나가게 함 — 짓눌린 물체가 순간이동하듯 뚫고 사라지는 문제의 원인으로 추정.
    constexpr float MAX_LINEAR_CORRECTION = 0.2f;
    float effectivePenetration = std::clamp(contact.penetration - SLOP, 0.0f, MAX_LINEAR_CORRECTION);
    float totalCorrection = effectivePenetration;   // 멀티패스 조기종료 판단용 누적치

    float separation = effectivePenetration / totalInvMass;
    Vector3 moveVector = contact.normal * separation;

    if (invMassA > 0) {
        Transform& trA = contact.colA->gameObject->transform;
        trA.SetPosition(trA.GetPosition() + (moveVector * invMassA));
    }
    if (invMassB > 0) {
        Transform& trB = contact.colB->gameObject->transform;
        trB.SetPosition(trB.GetPosition() - (moveVector * invMassB));
    }

    ///충격량 계산 — 접촉점(들) 기준 (선형 + 회전)

    Quaternion rotA = contact.colA->gameObject->transform.GetRotation();
    Quaternion rotB = contact.colB->gameObject->transform.GetRotation();
    bool spinA = (invMassA > 0.0f) && rbA->inverseInertiaLocal.MagnitudeSq() > 0.0f;
    bool spinB = (invMassB > 0.0f) && rbB->inverseInertiaLocal.MagnitudeSq() > 0.0f;

    // pointCount==0(대부분의 쌍)이면 기존처럼 contactPoint 하나로 1회만 순회 — 동작 동일
    int pointCount = contact.pointCount > 0 ? contact.pointCount : 1;
    // 다접점은 각 접점이 자기 접근속도를 온전히 죽이게 두고(임펄스 분할 금지), 대신 전체 접점을
    // 여러 번 반복 순회해 서로 수렴시킴(순차 임펄스). 1/N 분할+1패스는 접근속도가 ~30% 살아남아
    // 모서리가 계속 파고들고 위치보정이 에너지를 주입해 텀블링을 유발했음.
    int iterations = pointCount > 1 ? 4 : 1;

    for (int iter = 0; iter < iterations; ++iter)
    for (int p = 0; p < pointCount; ++p) {
        Vector3 contactPoint = contact.pointCount > 0 ? contact.points[p] : contact.contactPoint;

        // 지렛대 팔: 접촉점 - 질량중심(월드). 회전 가능(역관성>0)한 동적 바디만 의미 있음
        Vector3 rA = spinA ? contactPoint - (contact.colA->gameObject->transform.GetPosition() + rotA * rbA->centerOfMass) : Vector3::Zero();
        Vector3 rB = spinB ? contactPoint - (contact.colB->gameObject->transform.GetPosition() + rotB * rbB->centerOfMass) : Vector3::Zero();

        // 접촉점 상대속도: v + ω×r
        Vector3 velA = rbA ? rbA->linearVelocity : Vector3::Zero();
        Vector3 velB = rbB ? rbB->linearVelocity : Vector3::Zero();
        if (spinA) velA += Vector3::Cross(rbA->angularVelocity, rA);
        if (spinB) velB += Vector3::Cross(rbB->angularVelocity, rB);
        Vector3 relativeVelocity = velA - velB;

        float velAlongNormal = Vector3::Dot(relativeVelocity, contact.normal);
        if (velAlongNormal > 0) continue; // 이 접점은 이미 멀어지는 중

        // 유효질량 분모: Σ invMass + Σ n·((I⁻¹(r×n))×r)
        float angularTermN = 0.0f;
        if (spinA) {
            Vector3 raCrossN = Vector3::Cross(rA, contact.normal);
            angularTermN += Vector3::Dot(Vector3::Cross(ApplyInvInertia(rbA, rotA, raCrossN), rA), contact.normal);
        }
        if (spinB) {
            Vector3 rbCrossN = Vector3::Cross(rB, contact.normal);
            angularTermN += Vector3::Dot(Vector3::Cross(ApplyInvInertia(rbB, rotB, rbCrossN), rB), contact.normal);
        }

        float pointRestitution = (-velAlongNormal < BOUNCE_THRESHOLD) ? 0.0f : restitution;

        float j = -(1.0f + pointRestitution) * velAlongNormal;
        j /= (totalInvMass + angularTermN);

        Vector3 normalImpulse = contact.normal * j;
        totalCorrection += std::abs(j);

        if (rbA && invMassA > 0) rbA->SetVelocity(rbA->linearVelocity + normalImpulse * invMassA);
        if (rbB && invMassB > 0) rbB->SetVelocity(rbB->linearVelocity - normalImpulse * invMassB);
        if (spinA) { rbA->angularVelocity += ApplyInvInertia(rbA, rotA, Vector3::Cross(rA, normalImpulse)); rbA->isDirty = true; }
        if (spinB) { rbB->angularVelocity -= ApplyInvInertia(rbB, rotB, Vector3::Cross(rB, normalImpulse)); rbB->isDirty = true; }

        ///마찰 충격량 — 같은 접점 상대속도로 재계산

        velA = rbA ? rbA->linearVelocity : Vector3::Zero();
        velB = rbB ? rbB->linearVelocity : Vector3::Zero();
        if (spinA) velA += Vector3::Cross(rbA->angularVelocity, rA);
        if (spinB) velB += Vector3::Cross(rbB->angularVelocity, rB);
        relativeVelocity = velA - velB;

        // 접선 방향 (normal 성분 제거)
        Vector3 tangent = relativeVelocity - contact.normal * Vector3::Dot(relativeVelocity, contact.normal);
        float tangentLen = tangent.Magnitude();
        if (tangentLen < 0.0001f) continue; // 접선 속도 없으면 마찰 없음
        tangent = tangent * (1.0f / tangentLen);

        // 접선축 유효질량 (노멀과 동일 형태)
        float angularTermT = 0.0f;
        if (spinA) {
            Vector3 raCrossT = Vector3::Cross(rA, tangent);
            angularTermT += Vector3::Dot(Vector3::Cross(ApplyInvInertia(rbA, rotA, raCrossT), rA), tangent);
        }
        if (spinB) {
            Vector3 rbCrossT = Vector3::Cross(rB, tangent);
            angularTermT += Vector3::Dot(Vector3::Cross(ApplyInvInertia(rbB, rotB, rbCrossT), rB), tangent);
        }

        float velAlongTangent = Vector3::Dot(relativeVelocity, tangent);
        float jt = -velAlongTangent / (totalInvMass + angularTermT);

        // 쿨롱 마찰: |jt| <= friction * |j| 이면 정지 마찰, 초과하면 동적 마찰
        Vector3 frictionImpulse;
        if (std::abs(jt) <= j * staticFrictionC) {
            frictionImpulse = tangent * jt;           // 정지 마찰 (완전히 멈춤)
        } else {
            frictionImpulse = tangent * (-j * friction); // 동적 마찰 (미끄러짐)
        }

        if (rbA && invMassA > 0) rbA->SetVelocity(rbA->linearVelocity + frictionImpulse * invMassA);
        if (rbB && invMassB > 0) rbB->SetVelocity(rbB->linearVelocity - frictionImpulse * invMassB);

        // 구름 마찰 감쇠 — frictionImpulse는 "이 접점의 접선속도(선속도+회전기여분 합산)를 0으로" 만드는 크기라,
        // 그대로 각속도에 전부 되먹이면 이 점을 축으로 정상적으로 도는 중(피벗/텀블링)인 것까지 매 틱 제동을 걺.
        // 선속도(실제 미끄러짐)는 그대로 완전히 죽이되, 각속도 쪽은 약하게만 감쇠해서 회전은 계속 진행되게 함.
        constexpr float ROLLING_FRICTION_SCALE = 0.1f;
        if (spinA) rbA->angularVelocity += ApplyInvInertia(rbA, rotA, Vector3::Cross(rA, frictionImpulse)) * ROLLING_FRICTION_SCALE;
        if (spinB) rbB->angularVelocity -= ApplyInvInertia(rbB, rotB, Vector3::Cross(rB, frictionImpulse)) * ROLLING_FRICTION_SCALE;
    }

    return totalCorrection;
}
bool CollisionSolver::Raycast(const Ray& ray, Collider* collider, float maxDistance, RaycastHit& outHit) {
    if (!collider) return false;

    switch (collider->GetShapeType()) {
        case ColliderType::Sphere:
            return RaycastSphere(ray, static_cast<SphereCollider*>(collider), maxDistance, outHit);
        case ColliderType::Box:
            return RaycastBox(ray, static_cast<BoxCollider*>(collider), maxDistance, outHit);
        case ColliderType::Capsule:
            return RaycastCapsule(ray, static_cast<CapsuleCollider*>(collider), maxDistance, outHit);
        case ColliderType::Mesh:
            return RaycastMesh(ray, static_cast<MeshCollider*>(collider), maxDistance, outHit);
        default:
            return false;
    }
}
// endregion

// region Helper Functions
namespace {
    Rigidbody* GetSafeRigidbody(Collider* col) {
        if (!col || !col->gameObject) return nullptr;
        auto rbHandle = col->gameObject->GetComponent<Rigidbody>();
        // 핸들이 비어있으면(Rigidbody가 없으면)  nullptr 반환
        if (rbHandle.isNull()) return nullptr;

        return rbHandle.operator->();
    }

    void GetCapsuleSegment(CapsuleCollider* cap, Vector3& outTop, Vector3& outBottom) {
        Vector3 scale = cap->gameObject->transform.GetScale();
        Quaternion rot = cap->gameObject->transform.GetRotation();

        Vector3 scaledCenter = {cap->center.x * scale.x, cap->center.y * scale.y, cap->center.z * scale.z};
        Vector3 pos = cap->gameObject->transform.GetPosition() + (rot * scaledCenter);

        Vector3 dirVector = Vector3::Zero();
        float scaledHeight = cap->height;
        float rScale = 1.0f;

        if (cap->direction == 0) {
            dirVector = Vector3(1, 0, 0);
            scaledHeight *= scale.x;
            rScale = (std::max)(scale.y, scale.z);
        } else if (cap->direction == 1) {
            dirVector = Vector3(0, 1, 0);
            scaledHeight *= scale.y;
            rScale = (std::max)(scale.x, scale.z);
        } else if (cap->direction == 2) {
            dirVector = Vector3(0, 0, 1);
            scaledHeight *= scale.z;
            rScale = (std::max)(scale.x, scale.y);
        }
        dirVector = rot * dirVector;
        float scaledRadius = cap->radius * rScale;
        float distanceBetweenCenters = (std::max)(0.0f, scaledHeight - (scaledRadius * 2.0f));
        float halfDist = distanceBetweenCenters * 0.5f;

        outTop = pos + (dirVector * halfDist);
        outBottom = pos - (dirVector * halfDist);
    }

    // 두 선분(p1~q1, p2~q2) 사이의 가장 가까운 두 점(c1, c2)을 찾는 수학 함수
    // (실시간 물리 엔진의 표준 알고리즘인 Christer Ericson의 Real-Time Collision Detection 공식입니다)
    void ClosestPtSegmentSegment(Vector3 p1, Vector3 q1, Vector3 p2, Vector3 q2,
                                 Vector3& c1, Vector3& c2) {
        Vector3 d1 = q1 - p1;
        Vector3 d2 = q2 - p2;
        Vector3 r = p1 - p2;
        float a = Vector3::Dot(d1, d1);
        float e = Vector3::Dot(d2, d2);
        float f = Vector3::Dot(d2, r);

        float s = 0.0f, t = 0.0f;
        if (a <= EPSILON && e <= EPSILON) {
            s = t = 0.0f;
            c1 = p1; c2 = p2; return;
        }
        if (a <= EPSILON) { s = 0.0f; t = f / e; t = std::clamp(t, 0.0f, 1.0f); }
        else {
            float c = Vector3::Dot(d1, r);
            if (e <= EPSILON) { t = 0.0f; s = std::clamp(-c / a, 0.0f, 1.0f); }
            else {
                float b = Vector3::Dot(d1, d2);
                float denom = a * e - b * b;
                if (denom != 0.0f) s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
                else s = 0.0f;
                t = (b * s + f) / e;
                if (t < 0.0f) { t = 0.0f; s = std::clamp(-c / a, 0.0f, 1.0f); }
                else if (t > 1.0f) { t = 1.0f; s = std::clamp((b - c) / a, 0.0f, 1.0f); }
            }
        }
        c1 = p1 + d1 * s;
        c2 = p2 + d2 * t;
    }
    Vector3 ClosestPtPointSegment(Vector3 p, Vector3 a, Vector3 b) {
        Vector3 ab = b - a;
        // 선분 길이의 제곱이 0이면(a와 b가 같은 점) a 반환
        float sqLen = Vector3::Dot(ab, ab);
        if (sqLen <= EPSILON) return a;

        // p를 선분 ab에 투영시킨 비율(t) 계산
        float t = Vector3::Dot(p - a, ab) / sqLen;

        // t를 0과 1 사이로 강제 고정 (선분을 벗어나지 않도록)
        t = std::clamp(t, 0.0f, 1.0f);

        return a + (ab * t);
    }
    Vector3 ClosestPtPointTriangle(Vector3 p, Vector3 a, Vector3 b, Vector3 c) {
        Vector3 ab = b - a;
        Vector3 ac = c - a;
        Vector3 ap = p - a;

        float d1 = Vector3::Dot(ab, ap);
        float d2 = Vector3::Dot(ac, ap);
        if (d1 <= 0.0f && d2 <= 0.0f) return a; // 점 a 영역

        Vector3 bp = p - b;
        float d3 = Vector3::Dot(ab, bp);
        float d4 = Vector3::Dot(ac, bp);
        if (d3 >= 0.0f && d4 <= d3) return b; // 점 b 영역

        float vc = d1 * d4 - d3 * d2;
        if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
            float v = d1 / (d1 - d3);
            return a + (ab * v); // 선분 ab 영역
        }

        Vector3 cp = p - c;
        float d5 = Vector3::Dot(ab, cp);
        float d6 = Vector3::Dot(ac, cp);
        if (d6 >= 0.0f && d5 <= d6) return c; // 점 c 영역

        float vb = d5 * d2 - d1 * d6;
        if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
            float w = d2 / (d2 - d6);
            return a + (ac * w); // 선분 ac 영역
        }

        float va = d3 * d6 - d5 * d4;
        if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
            float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
            return b + ((c - b) * w); // 선분 bc 영역
        }

        // 위의 모든 엣지를 통과했다면 점은 삼각형 내부 면 위에 있음
        float denom = 1.0f / (va + vb + vc);
        float v = vb * denom;
        float w = vc * denom;
        return a + (ab * v) + (ac * w);
    }
    void ClosestPtSegmentTriangle(Vector3 segA, Vector3 segB, Vector3 triA, Vector3 triB, Vector3 triC,
                                  float& outMinDistSq, Vector3& outClosestSeg, Vector3& outClosestTri) {
        outMinDistSq = (std::numeric_limits<float>::max)();

        // 1. 캡슐의 두 끝점(Point) vs 삼각형 면(Triangle)
        Vector3 pts[2] = { segA, segB };
        for (int i = 0; i < 2; ++i) {
            Vector3 ptOnTri = ClosestPtPointTriangle(pts[i], triA, triB, triC);
            float distSq = (pts[i] - ptOnTri).MagnitudeSq();
            if (distSq < outMinDistSq) {
                outMinDistSq = distSq; outClosestSeg = pts[i]; outClosestTri = ptOnTri;
            }
        }

        // 2. 캡슐의 선분(Segment) vs 삼각형의 세 모서리(3 Edges)
        Vector3 edges[3][2] = { {triA, triB}, {triB, triC}, {triC, triA} };
        for (int i = 0; i < 3; ++i) {
            Vector3 c1, c2; // c1: 캡슐 선분 위의 점, c2: 삼각형 모서리 위의 점
            ClosestPtSegmentSegment(segA, segB, edges[i][0], edges[i][1], c1, c2);
            float distSq = (c1 - c2).MagnitudeSq();
            if (distSq < outMinDistSq) {
                outMinDistSq = distSq; outClosestSeg = c1; outClosestTri = c2;
            }
        }
    }

    ///SAT 분리축 정리 헬퍼 함수
    bool TestAxis(Vector3 axis, Vector3 centerDiff,
                  Vector3 aX, Vector3 aY, Vector3 aZ, Vector3 aExtents,
                  Vector3 bX, Vector3 bY, Vector3 bZ, Vector3 bExtents,
                  float& outPenetration) {
        float sqLen = axis.MagnitudeSq();
        if (sqLen < 0.0001f) return true; // 두 축이 평행해서 외적이 0이 된 경우 무시
        axis = axis / std::sqrt(sqLen);   // 축 정규화

        // A 박스를 축에 투영한 '그림자의 절반 길이(반지름)'
        float rA = aExtents.x * std::abs(Vector3::Dot(aX, axis)) +
                   aExtents.y * std::abs(Vector3::Dot(aY, axis)) +
                   aExtents.z * std::abs(Vector3::Dot(aZ, axis));

        // B 박스를 축에 투영한 '그림자의 절반 길이(반지름)'
        float rB = bExtents.x * std::abs(Vector3::Dot(bX, axis)) +
                   bExtents.y * std::abs(Vector3::Dot(bY, axis)) +
                   bExtents.z * std::abs(Vector3::Dot(bZ, axis));

        // 두 중심 사이의 거리를 축에 투영
        float dist = std::abs(Vector3::Dot(centerDiff, axis));

        outPenetration = (rA + rB) - dist;
        return outPenetration > 0.0f; // 0보다 커야 그림자가 겹친 것 (충돌 가능성 유지)
    }
}
// endregion

// region Raycast
namespace {
    // 월드 공간 구 레이캐스트 코어 — RaycastSphere/RaycastCapsule 끝단이 공용 (outHit.collider 세팅은 호출자 몫)
    bool RaycastSphereWorld(const Ray& ray, const Vector3& center, float radius, float maxDistance, RaycastHit& outHit) {
        Vector3 m = ray.origin - center;

        float b = Vector3::Dot(m, ray.direction);
        float c = Vector3::Dot(m, m) - (radius * radius);

        // 광선 시작점이 구 밖에 있고, 광선 방향이 구를 향하지 않으면 충돌 불가
        if (c > 0.0f && b > 0.0f) return false;

        float discr = b * b - c;
        // 판별식이 0보다 작으면 허근 (빗나감)
        if (discr < 0.0f) return false;

        // 가장 가까운 충돌 거리 (t)
        float t = -b - std::sqrt(discr);

        // 만약 광선 시작점이 구 내부라면 t는 음수이므로, 반대편 교차점을 사용
        if (t < 0.0f) t = -b + std::sqrt(discr);

        if (t > maxDistance) return false;

        outHit.distance = t;
        outHit.point = ray.origin + (ray.direction * t);
        outHit.normal = (outHit.point - center) / radius;

        return true;
    }
}

bool CollisionSolver::RaycastSphere(const Ray& ray, SphereCollider* sphere, float maxDistance, RaycastHit& outHit) {
    // center/radius에 트랜스폼 스케일·회전 적용 (RaycastBox와 동일 규약)
    const Transform& tr = sphere->gameObject->transform;
    Vector3 scale = tr.GetScale();
    Vector3 scaledCenter = { sphere->center.x * scale.x, sphere->center.y * scale.y, sphere->center.z * scale.z };
    Vector3 center = tr.GetPosition() + (tr.GetRotation() * scaledCenter);
    float radius = sphere->radius * (std::max)({std::abs(scale.x), std::abs(scale.y), std::abs(scale.z)});

    if (!RaycastSphereWorld(ray, center, radius, maxDistance, outHit)) return false;
    outHit.collider = sphere;
    return true;
}
bool CollisionSolver::RaycastBox(const Ray& ray, BoxCollider* box, float maxDistance, RaycastHit& outHit) {
    // center/size에 트랜스폼 스케일·회전 적용 (BoxVsBox와 동일 규약 — 스케일 빼먹으면 로우폴리 모델의
    // 거대 로컬 size가 그대로 월드 크기가 되어 레이 원점이 박스 내부로 판정되는(dist=0) 버그)
    Quaternion boxRot = box->gameObject->transform.GetRotation();
    Vector3 scale = box->gameObject->transform.GetScale();
    Vector3 scaledCenter = { box->center.x * scale.x, box->center.y * scale.y, box->center.z * scale.z };
    Vector3 boxPos = box->gameObject->transform.GetPosition() + (boxRot * scaledCenter);
    Vector3 extents = { box->size.x * scale.x * 0.5f, box->size.y * scale.y * 0.5f, box->size.z * scale.z * 0.5f };

    // 광선을 Box의 로컬 공간으로 역변환
    Quaternion invRot = boxRot;
    invRot.x = -invRot.x; invRot.y = -invRot.y; invRot.z = -invRot.z;

    Vector3 localOrigin = invRot * (ray.origin - boxPos);
    Vector3 localDir = invRot * ray.direction;

    float tMin = 0.0f;
    float tMax = maxDistance;
    Vector3 localNormal = Vector3::Zero();

    // X, Y, Z 3개의 축(Slab)에 대해 교차 구간(t1, t2)을 구함
    float* originPtr = &localOrigin.x;
    float* dirPtr = &localDir.x;
    float* extPtr = &extents.x;

    for (int i = 0; i < 3; ++i) {
        if (std::abs(dirPtr[i]) < EPSILON) {
            // 광선이 축과 평행한데 박스 밖에 있다면 충돌 불가
            if (originPtr[i] < -extPtr[i] || originPtr[i] > extPtr[i]) return false;
        } else {
            float ood = 1.0f / dirPtr[i];
            float t1 = (-extPtr[i] - originPtr[i]) * ood;
            float t2 = (extPtr[i] - originPtr[i]) * ood;

            if (t1 > t2) std::swap(t1, t2);

            if (t1 > tMin) {
                tMin = t1;
                // 법선 벡터 기록 (어느 축면에서 부딪혔는지)
                localNormal = Vector3::Zero();
                (&localNormal.x)[i] = (originPtr[i] < 0.0f) ? -1.0f : 1.0f;
            }
            if (t2 < tMax) tMax = t2;

            if (tMin > tMax) return false;
        }
    }

    outHit.collider = box;
    outHit.distance = tMin;
    outHit.point = ray.origin + (ray.direction * tMin);
    outHit.normal = boxRot * localNormal; // 법선을 다시 월드 회전으로 복구

    return true;
}
bool CollisionSolver::RaycastCapsule(const Ray& ray, CapsuleCollider* cap, float maxDistance, RaycastHit& outHit) {
    Vector3 capTop, capBottom;
    GetCapsuleSegment(cap, capTop, capBottom);

    // 반지름에 스케일 적용 (GetCapsuleSegment의 rScale과 동일 규약 — 축에 수직인 두 성분의 max)
    Vector3 capScale = cap->gameObject->transform.GetScale();
    float rScale = 1.0f;
    if (cap->direction == 0)      rScale = (std::max)(capScale.y, capScale.z);
    else if (cap->direction == 1) rScale = (std::max)(capScale.x, capScale.z);
    else                          rScale = (std::max)(capScale.x, capScale.y);
    float radius = cap->radius * rScale;

    Vector3 d = capBottom - capTop;
    float md = d.MagnitudeSq();
    if (md < EPSILON) { // 선분이 너무 짧으면 Sphere로 취급
        if (!RaycastSphereWorld(ray, capTop, radius, maxDistance, outHit)) return false;
        outHit.collider = cap;
        return true;
    }

    Vector3 m = ray.origin - capTop;
    Vector3 n = ray.direction;
    float md_inv = 1.0f / md;

    // 원기둥 교차 판정을 위한 2차 방정식 계수
    float nn = Vector3::Dot(n, n);
    float nd = Vector3::Dot(n, d);
    float dd = Vector3::Dot(d, d);
    float mn = Vector3::Dot(m, n);
    float md_dot = Vector3::Dot(m, d);

    float a = dd * nn - nd * nd;
    float b = dd * mn - nd * md_dot;
    float c = dd * Vector3::Dot(m, m) - md_dot * md_dot - radius * radius * dd;

    if (std::abs(a) < EPSILON) {
        // 광선이 원기둥 축과 평행한 경우, 캡슐 끝단 구(Sphere) 검사로 넘어감
        if (c > 0.0f) return false;
    } else {
        float discr = b * b - a * c;
        if (discr >= 0.0f) {
            float t = (-b - std::sqrt(discr)) / a;
            if (t >= 0.0f && t <= maxDistance) {
                // 충돌점(t)이 원기둥 선분 구간 [0, 1] 안에 있는지 확인
                float y = md_dot + t * nd;
                if (y >= 0.0f && y <= dd) {
                    outHit.collider = cap;
                    outHit.distance = t;
                    outHit.point = ray.origin + ray.direction * t;
                    // 원기둥 표면 법선 계산
                    Vector3 closestPtOnAxis = capTop + d * (y * md_inv);
                    outHit.normal = (outHit.point - closestPtOnAxis) / radius;
                    return true;
                }
            }
        }
    }

    // 원기둥 몸통에 안 맞았다면, 위/아래 끝단의 구(Sphere) 2개와 레이캐스트 테스트
    bool hitSphere = false;
    RaycastHit topHit, bottomHit;

    bool h1 = RaycastSphereWorld(ray, capTop, radius, maxDistance, topHit);
    bool h2 = RaycastSphereWorld(ray, capBottom, radius, maxDistance, bottomHit);

    if (h1 && h2) {
        outHit = (topHit.distance < bottomHit.distance) ? topHit : bottomHit;
        hitSphere = true;
    } else if (h1) {
        outHit = topHit; hitSphere = true;
    } else if (h2) {
        outHit = bottomHit; hitSphere = true;
    }

    if (hitSphere) {
        outHit.collider = cap; // 더미가 아닌 원본 콜라이더로 덮어쓰기
        return true;
    }

    return false;
}
bool CollisionSolver::RaycastMesh(const Ray& ray, MeshCollider* mesh, float maxDistance, RaycastHit& outHit) {
    const auto& verts = mesh->GetVertices();
    const auto& indices = mesh->GetTriangles();

    Vector3 mPos = mesh->gameObject->transform.GetPosition();
    Vector3 mScale = mesh->gameObject->transform.GetScale();
    Quaternion mRot = mesh->gameObject->transform.GetRotation();

    // 람다 함수 (월드 좌표 변환)
    auto LocalToWorld = [&](const Vector3& localV) -> Vector3 {
        Vector3 scaled = { localV.x * mScale.x, localV.y * mScale.y, localV.z * mScale.z };
        return (mRot * scaled) + mPos;
    };

    bool hit = false;
    float closestT = maxDistance;
    Vector3 bestNormal = Vector3::Zero();

    // Möller–Trumbore 교차 알고리즘
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 >= indices.size()) break;

        Vector3 v0 = LocalToWorld(verts[indices[i]]);
        Vector3 v1 = LocalToWorld(verts[indices[i+1]]);
        Vector3 v2 = LocalToWorld(verts[indices[i+2]]);

        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;
        Vector3 h = Vector3::Cross(ray.direction, edge2);
        float a = Vector3::Dot(edge1, h);

        // 광선이 삼각형 면과 평행하면(a가 0에 가까우면) 무시
        if (a > -EPSILON && a < EPSILON) continue;

        float f = 1.0f / a;
        Vector3 s = ray.origin - v0;
        float u = f * Vector3::Dot(s, h);

        if (u < 0.0f || u > 1.0f) continue;

        Vector3 q = Vector3::Cross(s, edge1);
        float v = f * Vector3::Dot(ray.direction, q);

        if (v < 0.0f || u + v > 1.0f) continue;

        // 교차 지점(t) 계산
        float t = f * Vector3::Dot(edge2, q);

        if (t > EPSILON && t < closestT) {
            closestT = t;
            // 삼각형의 법선(Normal) 계산: (v1-v0) x (v2-v0) 후 정규화
            bestNormal = Vector3::Cross(edge1, edge2);
            float nLen = bestNormal.Magnitude();
            if (nLen > EPSILON) bestNormal = bestNormal / nLen;

            // 광선이 뒤에서 맞은 경우(백페이스 컬링 방지용 법선 뒤집기)
            if (Vector3::Dot(ray.direction, bestNormal) > 0.0f) {
                bestNormal = bestNormal * -1.0f;
            }

            hit = true;
        }
    }

    if (hit) {
        outHit.collider = mesh;
        outHit.distance = closestT;
        outHit.point = ray.origin + (ray.direction * closestT);
        outHit.normal = bestNormal;
        return true;
    }

    return false;
}

bool CollisionSolver::OverlapSphere(const Vector3& center, float radius, Collider* collider, Contact& outContact) {
    if (!collider || !collider->gameObject) return false;

    // AABB를 이용한 1차 솎아내기 (Sphere-AABB 테스트는 빠름)
    AABB colAABB = collider->GetAABB();
    if (!colAABB.IntersectsSphere(center, radius)) return false;

    // 타입별로 정밀 거리 검사
    switch (collider->GetShapeType()) {
        case ColliderType::Sphere: {
            auto* sphere = static_cast<SphereCollider*>(collider);
            Vector3 sPos = sphere->gameObject->transform.GetPosition() + (sphere->gameObject->transform.GetRotation() * sphere->center);
            float sRadius = sphere->radius * sphere->gameObject->transform.GetScale().Max(); // 스케일 적용

            float distSq = (sPos - center).MagnitudeSq();
            float sumR = radius + sRadius;
            return distSq < (sumR * sumR);
        }
        case ColliderType::Box: {
            auto* box = static_cast<BoxCollider*>(collider);
            const auto& tr = box->gameObject->transform;
            Vector3 pos    = tr.GetPosition();
            Vector3 scale  = tr.GetScale();
            Quaternion rot = tr.GetRotation();

            // 박스 월드 중심 + half-extents (GetAABB와 동일 컨벤션)
            Vector3 worldCenter = pos + rot * Vector3(box->center.x * scale.x,
                                                      box->center.y * scale.y,
                                                      box->center.z * scale.z);
            Vector3 he = { box->size.x * scale.x * 0.5f,
                           box->size.y * scale.y * 0.5f,
                           box->size.z * scale.z * 0.5f };

            // sphere center를 박스 로컬 축에 투영
            Vector3 d  = center - worldCenter;
            float lx = Vector3::Dot(d, rot * Vector3(1, 0, 0));
            float ly = Vector3::Dot(d, rot * Vector3(0, 1, 0));
            float lz = Vector3::Dot(d, rot * Vector3(0, 0, 1));

            // half-extents로 클램프 → 최근접점까지 거리²
            float dx = lx - std::clamp(lx, -he.x, he.x);
            float dy = ly - std::clamp(ly, -he.y, he.y);
            float dz = lz - std::clamp(lz, -he.z, he.z);

            return (dx*dx + dy*dy + dz*dz) < (radius * radius);
        }
        case ColliderType::Capsule: {
            auto* cap = static_cast<CapsuleCollider*>(collider);
            Vector3 capTop, capBottom;
            GetCapsuleSegment(cap, capTop, capBottom);
            Vector3 closestPt = ClosestPtPointSegment(center, capTop, capBottom);
            return (closestPt - center).MagnitudeSq() < (radius + cap->radius) * (radius + cap->radius);
        }
        case ColliderType::Mesh: {
            auto* mesh = static_cast<MeshCollider*>(collider);

            const auto& verts = mesh->GetVertices();
            const auto& indices = mesh->GetTriangles();

            Vector3 mPos = mesh->gameObject->transform.GetPosition();
            Vector3 mScale = mesh->gameObject->transform.GetScale();
            Quaternion mRot = mesh->gameObject->transform.GetRotation();

            // 정점을 월드 좌표로 변환하는 람다
            auto LocalToWorld = [&](const Vector3& localV) -> Vector3 {
                Vector3 scaled = { localV.x * mScale.x, localV.y * mScale.y, localV.z * mScale.z };
                return (mRot * scaled) + mPos;
            };

            float radiusSq = radius * radius;

            // 메쉬의 모든 삼각형을 순회
            for (size_t i = 0; i < indices.size(); i += 3) {
                if (i + 2 >= indices.size()) break;

                Vector3 v0 = LocalToWorld(verts[indices[i]]);
                Vector3 v1 = LocalToWorld(verts[indices[i+1]]);
                Vector3 v2 = LocalToWorld(verts[indices[i+2]]);

                // 구의 중심과 삼각형 면 사이의 가장 가까운 점 찾기
                Vector3 closestPt = ClosestPtPointTriangle(center, v0, v1, v2);
                float distSq = (center - closestPt).MagnitudeSq();

                if (distSq <= radiusSq) {
                    return true;
                }
            }
            return false; // 끝까지 다 뒤졌는데 안 겹치면 false
        }
        default: return false;
    }
}

// endregion
bool CollisionSolver::SphereVsSphere(Collider *a, Collider *b, Contact &outContact) {
    auto* sphereA = static_cast<SphereCollider*>(a);
    auto* sphereB = static_cast<SphereCollider*>(b);

    Vector3 scaleA = sphereA->gameObject->transform.GetScale();
    Vector3 scaleB = sphereB->gameObject->transform.GetScale();
    float radiusA = sphereA->radius * (std::max)({scaleA.x, scaleA.y, scaleA.z});
    float radiusB = sphereB->radius * (std::max)({scaleB.x, scaleB.y, scaleB.z});

    Vector3 posA = sphereA->gameObject->transform.GetPosition() + (sphereA->gameObject->transform.GetRotation() * Vector3(sphereA->center.x * scaleA.x, sphereA->center.y * scaleA.y, sphereA->center.z * scaleA.z));
    Vector3 posB = sphereB->gameObject->transform.GetPosition() + (sphereB->gameObject->transform.GetRotation() * Vector3(sphereB->center.x * scaleB.x, sphereB->center.y * scaleB.y, sphereB->center.z * scaleB.z));

    Vector3 diff = posA - posB;
    float distSq = diff.MagnitudeSq();
    float sumRadius = radiusA + radiusB;

    if (distSq < sumRadius * sumRadius) {
        float dist = std::sqrt(distSq);
        outContact.colA = a; outContact.colB = b;
        outContact.rbA = GetSafeRigidbody(a);
outContact.rbB = GetSafeRigidbody(b);
        outContact.penetration = sumRadius - dist;
        outContact.normal = (dist > EPSILON) ? diff / dist : Vector3(0, 1, 0);
        // 접촉점: 두 표면점의 중점
        outContact.contactPoint = ((posA - outContact.normal * radiusA) + (posB + outContact.normal * radiusB)) * 0.5f;
        return true;
    }
    return false;
}

bool CollisionSolver::CapsuleVsCapsule(Collider *a, Collider *b, Contact &outContact) {
    auto* capA = static_cast<CapsuleCollider*>(a);
    auto* capB = static_cast<CapsuleCollider*>(b);

    Vector3 scaleA = capA->gameObject->transform.GetScale();
    Vector3 scaleB = capB->gameObject->transform.GetScale();
    float radiusA = capA->radius * (std::max)(scaleA.x, scaleA.z); // 방향이 Y라고 가정
    float radiusB = capB->radius * (std::max)(scaleB.x, scaleB.z);

    Vector3 aTop, aBottom, bTop, bBottom;
    GetCapsuleSegment(capA, aTop, aBottom);
    GetCapsuleSegment(capB, bTop, bBottom);

    Vector3 c1, c2;
    ClosestPtSegmentSegment(aTop, aBottom, bTop, bBottom, c1, c2);

    Vector3 diff = c1 - c2;
    float distSq = diff.MagnitudeSq();
    float sumRadius = radiusA + radiusB;

    if (distSq < sumRadius * sumRadius) {
        float dist = std::sqrt(distSq);
        outContact.colA = a; outContact.colB = b;
        outContact.rbA = GetSafeRigidbody(a);
outContact.rbB = GetSafeRigidbody(b);
        outContact.penetration = sumRadius - dist;
        outContact.normal = (dist > EPSILON) ? diff / dist : Vector3(0, 1, 0);
        // 접촉점: 축 최근접점에서 각자 표면으로 나간 점의 중점
        outContact.contactPoint = ((c1 - outContact.normal * radiusA) + (c2 + outContact.normal * radiusB)) * 0.5f;
        return true;
    }
    return false;
}

bool CollisionSolver::SphereVsCapsule(Collider *a, Collider *b, Contact &outContact) {
    auto* sphere = static_cast<SphereCollider*>(a);
    auto* cap = static_cast<CapsuleCollider*>(b);

    Vector3 scaleA = sphere->gameObject->transform.GetScale();
    Vector3 scaleB = cap->gameObject->transform.GetScale();
    float sRadius = sphere->radius * (std::max)({scaleA.x, scaleA.y, scaleA.z});
    float cRadius = cap->radius * (std::max)(scaleB.x, scaleB.z);

    Vector3 spherePos = sphere->gameObject->transform.GetPosition() + (sphere->gameObject->transform.GetRotation() * Vector3(sphere->center.x * scaleA.x, sphere->center.y * scaleA.y, sphere->center.z * scaleA.z));

    Vector3 capTop, capBottom;
    GetCapsuleSegment(cap, capTop, capBottom);

    Vector3 closestPt = ClosestPtPointSegment(spherePos, capTop, capBottom);
    Vector3 diff = spherePos - closestPt;
    float distSq = diff.MagnitudeSq();
    float sumRadius = sRadius + cRadius;

    if (distSq < sumRadius * sumRadius) {
        float dist = std::sqrt(distSq);
        outContact.colA = a; outContact.colB = b;
        outContact.rbA = GetSafeRigidbody(a);
outContact.rbB = GetSafeRigidbody(b);
        outContact.penetration = sumRadius - dist;
        outContact.normal = (dist > EPSILON) ? diff / dist : Vector3(0, 1, 0);
        // 접촉점: 구 표면점과 캡슐 표면점의 중점
        outContact.contactPoint = ((spherePos - outContact.normal * sRadius) + (closestPt + outContact.normal * cRadius)) * 0.5f;
        return true;
    }
    return false;
}

bool CollisionSolver::SphereVsMesh(Collider *a, Collider *b, Contact &outContact) {
    auto* sphere = static_cast<SphereCollider*>(a);
    auto* mesh = static_cast<MeshCollider*>(b);

    Vector3 scaleA = sphere->gameObject->transform.GetScale();
    float sRadius = sphere->radius * (std::max)({scaleA.x, scaleA.y, scaleA.z});
    float radiusSq = sRadius * sRadius;

    Vector3 spherePos = sphere->gameObject->transform.GetPosition() + (sphere->gameObject->transform.GetRotation() * Vector3(sphere->center.x * scaleA.x, sphere->center.y * scaleA.y, sphere->center.z * scaleA.z));
    AABB sphereAABB = sphere->GetAABB();

    const auto& verts = mesh->GetVertices();
    const auto& indices = mesh->GetTriangles();

    Vector3 mPos = mesh->gameObject->transform.GetPosition();
    Vector3 mScale = mesh->gameObject->transform.GetScale();
    Quaternion mRot = mesh->gameObject->transform.GetRotation();

    auto LocalToWorld = [&](const Vector3& localV) -> Vector3 {
        Vector3 scaled = { localV.x * mScale.x, localV.y * mScale.y, localV.z * mScale.z };
        return (mRot * scaled) + mPos;
    };

    bool hasCollision = false;
    float maxPenetration = -1.0f;
    Vector3 bestNormal = Vector3::Zero();
    Vector3 bestPoint = Vector3::Zero();

    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 >= indices.size()) break;

        Vector3 v0 = LocalToWorld(verts[indices[i]]);
        Vector3 v1 = LocalToWorld(verts[indices[i+1]]);
        Vector3 v2 = LocalToWorld(verts[indices[i+2]]);

        Vector3 triMin = { (std::min)({v0.x, v1.x, v2.x}), (std::min)({v0.y, v1.y, v2.y}), (std::min)({v0.z, v1.z, v2.z}) };
        Vector3 triMax = { (std::max)({v0.x, v1.x, v2.x}), (std::max)({v0.y, v1.y, v2.y}), (std::max)({v0.z, v1.z, v2.z}) };
        AABB triAABB = { triMin, triMax };

        if (!sphereAABB.Intersects(triAABB)) continue;

        Vector3 closestPt = ClosestPtPointTriangle(spherePos, v0, v1, v2);
        Vector3 diff = spherePos - closestPt;
        float distSq = diff.MagnitudeSq();

        if (distSq < radiusSq) {
            float dist = std::sqrt(distSq);
            float penetration = sRadius - dist;

            if (penetration > maxPenetration) {
                maxPenetration = penetration;
                bestNormal = (dist > EPSILON) ? (diff / dist) : Vector3(0, 1, 0);
                bestPoint = closestPt;   // 최심 삼각형 위의 최근접점 = 접촉점
                hasCollision = true;
            }
        }
    }

    if (hasCollision) {
        outContact.colA = a; outContact.colB = b;
        outContact.rbA = GetSafeRigidbody(a);
outContact.rbB = GetSafeRigidbody(b);
        outContact.penetration = maxPenetration;
        outContact.normal = bestNormal;
        outContact.contactPoint = bestPoint;
        return true;
    }
    return false;
}

bool CollisionSolver::CapsuleVsMesh(Collider *a, Collider *b, Contact &outContact) {
    auto* cap = static_cast<CapsuleCollider*>(a);
    auto* mesh = static_cast<MeshCollider*>(b);

    Vector3 scaleA = cap->gameObject->transform.GetScale();
    float cRadius = cap->radius * (std::max)(scaleA.x, scaleA.z);
    float radiusSq = cRadius * cRadius;

    Vector3 capTop, capBottom;
    GetCapsuleSegment(cap, capTop, capBottom);
    AABB capAABB = cap->GetAABB();

    const auto& verts = mesh->GetVertices();
    const auto& indices = mesh->GetTriangles();

    Vector3 mPos = mesh->gameObject->transform.GetPosition();
    Vector3 mScale = mesh->gameObject->transform.GetScale();
    Quaternion mRot = mesh->gameObject->transform.GetRotation();

    auto LocalToWorld = [&](const Vector3& localV) -> Vector3 {
        Vector3 scaled = { localV.x * mScale.x, localV.y * mScale.y, localV.z * mScale.z };
        return (mRot * scaled) + mPos;
    };

    bool hasCollision = false;
    float maxPenetration = -1.0f;
    Vector3 bestNormal = Vector3::Zero();
    Vector3 bestPoint = Vector3::Zero();

    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 >= indices.size()) break;

        Vector3 v0 = LocalToWorld(verts[indices[i]]);
        Vector3 v1 = LocalToWorld(verts[indices[i+1]]);
        Vector3 v2 = LocalToWorld(verts[indices[i+2]]);

        Vector3 triMin = { (std::min)({v0.x, v1.x, v2.x}), (std::min)({v0.y, v1.y, v2.y}), (std::min)({v0.z, v1.z, v2.z}) };
        Vector3 triMax = { (std::max)({v0.x, v1.x, v2.x}), (std::max)({v0.y, v1.y, v2.y}), (std::max)({v0.z, v1.z, v2.z}) };

        if (capAABB.max.x < triMin.x || capAABB.min.x > triMax.x) continue;
        if (capAABB.max.y < triMin.y || capAABB.min.y > triMax.y) continue;
        if (capAABB.max.z < triMin.z || capAABB.min.z > triMax.z) continue;
        Vector3 triNormal = Vector3::Cross(v1 - v0, v2 - v0);
        float nLen = triNormal.Magnitude();
        if (nLen > EPSILON) triNormal = triNormal / nLen;
        else continue;

        float distSq;
        Vector3 closestSeg, closestTri;
        ClosestPtSegmentTriangle(capTop, capBottom, v0, v1, v2, distSq, closestSeg, closestTri);

        if (distSq < radiusSq) {
            float dist = std::sqrt(distSq);
            float penetration = 0.0f;
            Vector3 normal = Vector3::Zero();

            if (dist > EPSILON) {
                // 고스트 엣지 방지: 최근접점 방향(closestSeg-closestTri) 대신 삼각형 면법선을 캡슐 쪽으로 보정해서 사용.
                // 이음새에서 승자 삼각형이 바뀔 때마다 최근접점 방향도 같이 튀면서 옆으로 토크가 걸려
                // 텀블링을 유발했던 것으로 추정(BoxVsMesh에 이미 적용된 것과 동일한 조치).
                Vector3 capCenter = (capTop + capBottom) * 0.5f;
                float side = Vector3::Dot(capCenter - v0, triNormal);
                normal = (side >= 0.0f) ? triNormal : (triNormal * -1.0f);
                penetration = cRadius - dist;
            } else {
                // 깊은 관통: 캡슐 중심이 있는 쪽으로 밀어냄 (양면)
                Vector3 capCenter = (capTop + capBottom) * 0.5f;
                float side = Vector3::Dot(capCenter - v0, triNormal);
                normal = (side >= 0.0f) ? triNormal : (triNormal * -1.0f);

                float pTop = Vector3::Dot(capTop - v0, normal);
                float pBot = Vector3::Dot(capBottom - v0, normal);
                float minProj = (std::min)(pTop, pBot);
                penetration = cRadius - minProj;
            }

            if (penetration > maxPenetration) {
                maxPenetration = penetration;
                bestNormal = normal;
                bestPoint = closestTri;   // 삼각형 위의 최근접점 = 접촉점 (깊은 관통 시에도 유효)
                hasCollision = true;
            }
        }
    }

    if (hasCollision) {
        outContact.colA = a; outContact.colB = b;
        outContact.rbA = GetSafeRigidbody(a);
outContact.rbB = GetSafeRigidbody(b);
        outContact.penetration = maxPenetration;
        outContact.normal = bestNormal;
        outContact.contactPoint = bestPoint;
        return true;
    }
    return false;
}

bool CollisionSolver::SphereVsBox(Collider *a, Collider *b, Contact &outContact) {
    auto* sphere = static_cast<SphereCollider*>(a);
    auto* box = static_cast<BoxCollider*>(b);

    Vector3 scaleS = sphere->gameObject->transform.GetScale();
    Vector3 scaleB = box->gameObject->transform.GetScale();

    float sRadius = sphere->radius * (std::max)({scaleS.x, scaleS.y, scaleS.z});
    Vector3 spherePos = sphere->gameObject->transform.GetPosition() + (sphere->gameObject->transform.GetRotation() * Vector3(sphere->center.x * scaleS.x, sphere->center.y * scaleS.y, sphere->center.z * scaleS.z));
    Vector3 boxPos = box->gameObject->transform.GetPosition() + (box->gameObject->transform.GetRotation() * Vector3(box->center.x * scaleB.x, box->center.y * scaleB.y, box->center.z * scaleB.z));

    Quaternion boxRot = box->gameObject->transform.GetRotation();
    Vector3 extents = {box->size.x * scaleB.x * 0.5f, box->size.y * scaleB.y * 0.5f, box->size.z * scaleB.z * 0.5f};

    Quaternion invRot = boxRot;
    invRot.x = -invRot.x; invRot.y = -invRot.y; invRot.z = -invRot.z;
    Vector3 localSpherePos = invRot * (spherePos - boxPos);

    Vector3 ptOnBox = {
        std::clamp(localSpherePos.x, -extents.x, extents.x),
        std::clamp(localSpherePos.y, -extents.y, extents.y),
        std::clamp(localSpherePos.z, -extents.z, extents.z)
    };

    Vector3 localDiff = localSpherePos - ptOnBox;
    float distSq = localDiff.MagnitudeSq();
    float radiusSq = sRadius * sRadius;

    if (distSq < radiusSq) {
        float dist = std::sqrt(distSq);
        Vector3 localNormal;
        float penetration;

        if (dist > EPSILON) {
            localNormal = localDiff / dist;
            penetration = sRadius - dist;
        } else {
            float dx = extents.x - std::abs(localSpherePos.x);
            float dy = extents.y - std::abs(localSpherePos.y);
            float dz = extents.z - std::abs(localSpherePos.z);

            if (dx < dy && dx < dz) {
                localNormal = Vector3((localSpherePos.x > 0) ? 1.0f : -1.0f, 0, 0);
                penetration = sRadius + dx;
            } else if (dy < dx && dy < dz) {
                localNormal = Vector3(0, (localSpherePos.y > 0) ? 1.0f : -1.0f, 0);
                penetration = sRadius + dy;
            } else {
                localNormal = Vector3(0, 0, (localSpherePos.z > 0) ? 1.0f : -1.0f);
                penetration = sRadius + dz;
            }
        }

        outContact.colA = a;
        outContact.colB = b;
        outContact.rbA = GetSafeRigidbody(a);
outContact.rbB = GetSafeRigidbody(b);
        outContact.penetration = penetration;
        outContact.normal = boxRot * localNormal;
        // 접촉점: 박스 표면 최근접점(월드). 구 중심이 박스 내부면 구 중심 사용
        outContact.contactPoint = (dist > EPSILON) ? (boxPos + (boxRot * ptOnBox)) : spherePos;

        return true;
    }
    return false;
}

bool CollisionSolver::BoxVsCapsule(Collider *a, Collider *b, Contact &outContact) {
    auto* box = static_cast<BoxCollider*>(a);
    auto* cap = static_cast<CapsuleCollider*>(b);

    Vector3 scaleB = box->gameObject->transform.GetScale();
    Vector3 scaleC = cap->gameObject->transform.GetScale();

    Vector3 boxPos = box->gameObject->transform.GetPosition() + (box->gameObject->transform.GetRotation() * Vector3(box->center.x * scaleB.x, box->center.y * scaleB.y, box->center.z * scaleB.z));
    Quaternion boxRot = box->gameObject->transform.GetRotation();
    Vector3 extents = {box->size.x * scaleB.x * 0.5f, box->size.y * scaleB.y * 0.5f, box->size.z * scaleB.z * 0.5f};

    float cRadius = cap->radius * (std::max)(scaleC.x, scaleC.z);

    Vector3 capTop, capBottom;
    GetCapsuleSegment(cap, capTop, capBottom);

    Quaternion invRot = boxRot;
    invRot.x = -invRot.x; invRot.y = -invRot.y; invRot.z = -invRot.z;

    Vector3 localTop = invRot * (capTop - boxPos);
    Vector3 localBottom = invRot * (capBottom - boxPos);

    auto ClampToAABB = [&](const Vector3& p) -> Vector3 {
        return {
            std::clamp(p.x, -extents.x, extents.x),
            std::clamp(p.y, -extents.y, extents.y),
            std::clamp(p.z, -extents.z, extents.z)
        };
    };

    Vector3 ptOnSeg = (localTop + localBottom) * 0.5f;
    Vector3 ptOnBox;

    for (int i = 0; i < 3; ++i) {
        ptOnBox = ClampToAABB(ptOnSeg);
        ptOnSeg = ClosestPtPointSegment(ptOnBox, localTop, localBottom);
    }

    Vector3 localDiff = ptOnBox - ptOnSeg;
    float distSq = localDiff.MagnitudeSq();
    float radiusSq = cRadius * cRadius;

    if (distSq < radiusSq) {
        float dist = std::sqrt(distSq);
        Vector3 localNormal;
        float penetration;

        if (dist > EPSILON) {
            localNormal = localDiff / dist;
            penetration = cRadius - dist;
        } else {
            float dx = extents.x - std::abs(ptOnBox.x);
            float dy = extents.y - std::abs(ptOnBox.y);
            float dz = extents.z - std::abs(ptOnBox.z);

            if (dx < dy && dx < dz) {
                localNormal = Vector3((ptOnBox.x > 0) ? -1.0f : 1.0f, 0, 0);
                penetration = cRadius + dx;
            } else if (dy < dx && dy < dz) {
                localNormal = Vector3(0, (ptOnBox.y > 0) ? -1.0f : 1.0f, 0);
                penetration = cRadius + dy;
            } else {
                localNormal = Vector3(0, 0, (ptOnBox.z > 0) ? -1.0f : 1.0f);
                penetration = cRadius + dz;
            }
        }

        outContact.colA = a;
        outContact.colB = b;
        outContact.rbA = GetSafeRigidbody(a);
outContact.rbB = GetSafeRigidbody(b);
        outContact.penetration = penetration;
        outContact.normal = boxRot * localNormal;
        // 접촉점: 박스 표면 최근접점(박스 로컬 → 월드)
        outContact.contactPoint = boxPos + (boxRot * ptOnBox);

        return true;
    }

    return false;
}

bool CollisionSolver::BoxVsBox(Collider *a, Collider *b, Contact &outContact) {
    auto* boxA = static_cast<BoxCollider*>(a);
    auto* boxB = static_cast<BoxCollider*>(b);

    const Transform& trA = boxA->gameObject->transform;
    const Transform& trB = boxB->gameObject->transform;

    Vector3 scaleA = trA.GetScale();
    Vector3 scaleB = trB.GetScale();

    Quaternion rotA = trA.GetRotation();
    Quaternion rotB = trB.GetRotation();

    // ====================================================================
    // 🚨 [수정 1] 중심점(Center) 계산 버그 수정!
    // 단순 더하기가 아니라, Center도 스케일을 곱하고 회전시켜야 진짜 월드 위치가 됩니다.
    // ====================================================================
    Vector3 scaledCenterA = { boxA->center.x * scaleA.x, boxA->center.y * scaleA.y, boxA->center.z * scaleA.z };
    Vector3 scaledCenterB = { boxB->center.x * scaleB.x, boxB->center.y * scaleB.y, boxB->center.z * scaleB.z };

    Vector3 posA = trA.GetPosition() + (rotA * scaledCenterA);
    Vector3 posB = trB.GetPosition() + (rotB * scaledCenterB);

    // ====================================================================
    // 🚨 [수정 2] 박스 크기(Extents) 스케일 적용!
    // ====================================================================
    Vector3 extA = {
        boxA->size.x * scaleA.x * 0.5f,
        boxA->size.y * scaleA.y * 0.5f,
        boxA->size.z * scaleA.z * 0.5f
    };
    Vector3 extB = {
        boxB->size.x * scaleB.x * 0.5f,
        boxB->size.y * scaleB.y * 0.5f,
        boxB->size.z * scaleB.z * 0.5f
    };

    // 각 박스의 로컬 X, Y, Z 축을 월드 벡터로 추출
    Vector3 aX = rotA * Vector3(1, 0, 0);
    Vector3 aY = rotA * Vector3(0, 1, 0);
    Vector3 aZ = rotA * Vector3(0, 0, 1);

    Vector3 bX = rotB * Vector3(1, 0, 0);
    Vector3 bY = rotB * Vector3(0, 1, 0);
    Vector3 bZ = rotB * Vector3(0, 0, 1);

    Vector3 centerDiff = posA - posB; // B에서 A로 향하는 중심 벡터

    // 검사해야 할 15개의 분리축 (Separating Axes)
    Vector3 axes[15] = {
        aX, aY, aZ, // A의 면 법선
        bX, bY, bZ, // B의 면 법선
        Vector3::Cross(aX, bX), Vector3::Cross(aX, bY), Vector3::Cross(aX, bZ),
        Vector3::Cross(aY, bX), Vector3::Cross(aY, bY), Vector3::Cross(aY, bZ),
        Vector3::Cross(aZ, bX), Vector3::Cross(aZ, bY), Vector3::Cross(aZ, bZ)
    };

    float minPenetration = (std::numeric_limits<float>::max)();
    Vector3 bestAxis = Vector3::Zero();
    int bestAxisIndex = -1;

    // 15개 축에 대해 그림자가 겹치는지 테스트
    for (int i = 0; i < 15; ++i) {
        float penetration;
        if (!TestAxis(axes[i], centerDiff, aX, aY, aZ, extA, bX, bY, bZ, extB, penetration)) {
            // 단 하나의 축이라도 그림자가 안 겹치면 절대 충돌한 게 아님! (Early Exit)
            return false;
        }

        // 가장 얕게 파고든 축이 '진짜 부딪힌 방향(Normal)'입니다.
        if (penetration < minPenetration) {
            minPenetration = penetration;
            bestAxis = axes[i];
            bestAxisIndex = i;
        }
    }

    // 법선 방향이 항상 B에서 A를 향하도록 보정
    if (Vector3::Dot(bestAxis, centerDiff) < 0.0f) {
        bestAxis = bestAxis * -1.0f;
    }

    // 축 정규화 (TestAxis에서 정규화하기 전의 벡터가 들어왔을 수 있으므로)
    float axLen = bestAxis.Magnitude();
    if (axLen > 0.0001f) bestAxis = bestAxis / axLen;
    else bestAxis = Vector3(0, 1, 0);

    // 접촉점: 충돌축 방향 서포트(최심 꼭짓점) 중점 — 단일 접점 대표.
    // 면-면 안정 적층까지 필요해지면 face clipping 매니폴드(다접점)로 승격할 것
    auto BoxSupport = [](const Vector3& pos, const Vector3& ux, const Vector3& uy, const Vector3& uz,
                         const Vector3& ext, const Vector3& dir) {
        return pos + ux * (ext.x * (Vector3::Dot(ux, dir) >= 0.0f ? 1.0f : -1.0f))
                   + uy * (ext.y * (Vector3::Dot(uy, dir) >= 0.0f ? 1.0f : -1.0f))
                   + uz * (ext.z * (Vector3::Dot(uz, dir) >= 0.0f ? 1.0f : -1.0f));
    };
    Vector3 suppA = BoxSupport(posA, aX, aY, aZ, extA, bestAxis * -1.0f);   // A에서 B쪽으로 가장 깊은 점
    Vector3 suppB = BoxSupport(posB, bX, bY, bZ, extB, bestAxis);           // B에서 A쪽으로 가장 깊은 점

    outContact.colA = a;
    outContact.colB = b;
    outContact.rbA = GetSafeRigidbody(a);
    outContact.rbB = GetSafeRigidbody(b);
    outContact.penetration = minPenetration;
    outContact.normal = bestAxis;
    outContact.contactPoint = (suppA + suppB) * 0.5f;
    outContact.pointCount = 0;   // 기본은 단일접점(모서리-모서리 등)

    // bestAxisIndex < 6 = 면 법선이 이겼다 = 면-면 접촉(박스가 얹혀있는 케이스) → 다접점 매니폴드로 승격
    // 6 이상(외적축)은 모서리-모서리 접촉이라 접점이 진짜 하나뿐이므로 위 단일점 그대로 둠
    if (bestAxisIndex >= 0 && bestAxisIndex < 6) {
        bool refIsA = bestAxisIndex < 3;
        Vector3 refPos   = refIsA ? posA : posB;
        Vector3 refX     = refIsA ? aX : bX;
        Vector3 refY     = refIsA ? aY : bY;
        Vector3 refZ     = refIsA ? aZ : bZ;
        Vector3 refExt   = refIsA ? extA : extB;

        Vector3 incPos   = refIsA ? posB : posA;
        Vector3 incX     = refIsA ? bX : aX;
        Vector3 incY     = refIsA ? bY : aY;
        Vector3 incZ     = refIsA ? bZ : aZ;
        Vector3 incExt   = refIsA ? extB : extA;

        // 인시던트 박스에서 bestAxis와 가장 반대로 마주보는 면 하나를 고른다
        Vector3 dir = bestAxis * -1.0f;   // 인시던트 입장에서 "레퍼런스를 향한" 방향
        float dotX = std::abs(Vector3::Dot(incX, dir));
        float dotY = std::abs(Vector3::Dot(incY, dir));
        float dotZ = std::abs(Vector3::Dot(incZ, dir));

        // 두 면이 실제로 거의 평행(플러시)할 때만 4점 매니폴드로 승격 — BoxVsMesh에 이미 있는 동일한 안전장치.
        // SAT 승리축이 면법선이어도 실제로는 모서리/꼭짓점만 닿은 기울어진 접촉일 수 있어서,
        // 그런 경우까지 4점으로 승격하면 가짜 접점이 생겨 넘어지는 중인 박스를 인위적으로 멈춰 세움.
        // 0.95(약 18도)는 너무 느슨해서 아직 다 안 넘어간 상태에서도 승격돼버렸음 — 0.999(약 2.5도)로 강화.
        if ((std::max)({dotX, dotY, dotZ}) > 0.9999f) {
            Vector3 faceNormalAxis, faceU, faceV;
            float faceNormalExt, faceUExt, faceVExt;
            if (dotX >= dotY && dotX >= dotZ) {
                faceNormalAxis = incX; faceNormalExt = incExt.x;
                faceU = incY; faceUExt = incExt.y;
                faceV = incZ; faceVExt = incExt.z;
            } else if (dotY >= dotX && dotY >= dotZ) {
                faceNormalAxis = incY; faceNormalExt = incExt.y;
                faceU = incX; faceUExt = incExt.x;
                faceV = incZ; faceVExt = incExt.z;
            } else {
                faceNormalAxis = incZ; faceNormalExt = incExt.z;
                faceU = incX; faceUExt = incExt.x;
                faceV = incY; faceVExt = incExt.y;
            }
            if (Vector3::Dot(faceNormalAxis, dir) < 0.0f) faceNormalAxis = faceNormalAxis * -1.0f;

            Vector3 faceCenter = incPos + faceNormalAxis * faceNormalExt;
            Vector3 corners[4] = {
                faceCenter + faceU * faceUExt + faceV * faceVExt,
                faceCenter + faceU * faceUExt - faceV * faceVExt,
                faceCenter - faceU * faceUExt + faceV * faceVExt,
                faceCenter - faceU * faceUExt - faceV * faceVExt,
            };

            // 각 꼭짓점을 레퍼런스 박스 로컬 좌표로 투영 후, 레퍼런스 박스 범위 안으로 클램프
            // (정식 Sutherland-Hodgman 클리핑의 근사 — 대부분의 평면 적층 케이스에서 충분히 안정적)
            for (int i = 0; i < 4; ++i) {
                Vector3 rel = corners[i] - refPos;
                float u = std::clamp(Vector3::Dot(rel, refX), -refExt.x, refExt.x);
                float v = std::clamp(Vector3::Dot(rel, refY), -refExt.y, refExt.y);
                float w = std::clamp(Vector3::Dot(rel, refZ), -refExt.z, refExt.z);
                outContact.points[i] = refPos + refX * u + refY * v + refZ * w;
            }
            outContact.pointCount = 4;
        }
    }
    // bestAxisIndex >= 6 = 외적축(모서리-모서리 접촉) 승리 — 두 모서리가 근접 평행하면 겹치는 구간이
    // 점 하나가 아니라 선분이므로, 그 구간의 양 끝점을 2점 매니폴드로 사용(완전히 스큐인 경우는
    // 겹침이 사실상 0이라 자연스럽게 단일점과 동일해짐).
    else if (bestAxisIndex >= 6) {
        int edgeIdx = bestAxisIndex - 6;
        int ai = edgeIdx / 3;   // A의 모서리 방향 축 인덱스(0=aX,1=aY,2=aZ)
        int bi = edgeIdx % 3;   // B의 모서리 방향 축 인덱스

        Vector3 aAxesArr[3] = { aX, aY, aZ };
        float aExtArr[3] = { extA.x, extA.y, extA.z };
        Vector3 bAxesArr[3] = { bX, bY, bZ };
        float bExtArr[3] = { extB.x, extB.y, extB.z };

        // 두 모서리가 실제로 거의 평행/반평행할 때만 "겹치는 구간(선분)"이 기하학적으로 의미 있음.
        // 각도가 있는 진짜 스큐(skew) 교차는 최근접점이 하나뿐인데, 이 체크 없이 투영-겹침만으로
        // 2점을 만들면 실존하지 않는 가짜 접점이 생겨서 큰 위치보정이 걸릴 수 있음
        // (박스+박스+바닥 메쉬가 동시에 얽힐 때 순간적으로 튕겨나가 뚫리는 원인으로 추정).
        float edgeAlignment = std::abs(Vector3::Dot(aAxesArr[ai], bAxesArr[bi]));
        if (edgeAlignment > 0.9f) {
        auto GetBoxEdge = [](const Vector3& pos, const Vector3 axes[3], const float ext[3], int edgeAxis,
                              const Vector3& towardOther, Vector3& p0, Vector3& p1) {
            Vector3 base = pos;
            for (int k = 0; k < 3; ++k) {
                if (k == edgeAxis) continue;
                float sign = (Vector3::Dot(axes[k], towardOther) >= 0.0f) ? 1.0f : -1.0f;
                base = base + axes[k] * (ext[k] * sign);
            }
            p0 = base + axes[edgeAxis] * ext[edgeAxis];
            p1 = base - axes[edgeAxis] * ext[edgeAxis];
        };

        Vector3 aP0, aP1, bP0, bP1;
        GetBoxEdge(posA, aAxesArr, aExtArr, ai, (posB - posA), aP0, aP1);
        GetBoxEdge(posB, bAxesArr, bExtArr, bi, (posA - posB), bP0, bP1);

        // A의 모서리 방향으로 두 모서리를 투영해서 겹치는 구간을 구함(근접 평행 가정 — 완전 스큐면 겹침 0)
        Vector3 edgeDir = aP1 - aP0;
        float edgeLen = edgeDir.Magnitude();
        if (edgeLen > EPSILON) {
            edgeDir = edgeDir / edgeLen;
            float bProj0 = Vector3::Dot(bP0 - aP0, edgeDir);
            float bProj1 = Vector3::Dot(bP1 - aP0, edgeDir);
            float overlapMin = (std::max)(0.0f, (std::min)(bProj0, bProj1));
            float overlapMax = (std::min)(edgeLen, (std::max)(bProj0, bProj1));

            if (overlapMax - overlapMin > 0.001f) {
                outContact.points[0] = aP0 + edgeDir * overlapMin;
                outContact.points[1] = aP0 + edgeDir * overlapMax;
                outContact.pointCount = 2;
            }
        }
        }   // edgeAlignment > 0.9f
    }

    return true;
}

bool CollisionSolver::BoxVsMesh(Collider *a, Collider *b, Contact &outContact) {
    auto* box = static_cast<BoxCollider*>(a);
    auto* mesh = static_cast<MeshCollider*>(b);


    Vector3 scaleB = box->gameObject->transform.GetScale();
    Vector3 boxPos = box->gameObject->transform.GetPosition() + (box->gameObject->transform.GetRotation() * Vector3(box->center.x * scaleB.x, box->center.y * scaleB.y, box->center.z * scaleB.z));
    Quaternion boxRot = box->gameObject->transform.GetRotation();
    Vector3 extents = {box->size.x * scaleB.x * 0.5f, box->size.y * scaleB.y * 0.5f, box->size.z * scaleB.z * 0.5f};
    AABB boxAABB = box->GetAABB(); // Mid-phase 체크용

    const auto& verts = mesh->GetVertices();
    const auto& indices = mesh->GetTriangles();

    Vector3 mPos = mesh->gameObject->transform.GetPosition();
    Vector3 mScale = mesh->gameObject->transform.GetScale();
    Quaternion mRot = mesh->gameObject->transform.GetRotation();

    // 박스의 역회전 쿼터니언 (켤레 복소수)
    Quaternion invBoxRot = boxRot;
    invBoxRot.x = -invBoxRot.x; invBoxRot.y = -invBoxRot.y; invBoxRot.z = -invBoxRot.z;

    // 메쉬의 로컬 정점을 월드로 바꾼 뒤, 곧바로 박스의 로컬 공간으로 밀어넣는 미친 최적화 함수
    auto MeshToBoxLocal = [&](const Vector3& meshLocalV) -> Vector3 {
        Vector3 scaled = { meshLocalV.x * mScale.x, meshLocalV.y * mScale.y, meshLocalV.z * mScale.z };
        Vector3 worldV = (mRot * scaled) + mPos;
        return invBoxRot * (worldV - boxPos);
    };

    bool hasCollision = false;
    float maxPenetration = -1.0f;
    Vector3 bestWorldNormal = Vector3::Zero();
    Vector3 bestPlanePoint = Vector3::Zero();   // 승리한 삼각형 위의 한 점 (평면 투영용)

    // 삼각형 단위로 순회
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 >= indices.size()) break;

        // =======================================================
        // 1. [Mid-phase] 월드 공간에서 AABB 러프 체크 (안 겹치면 즉시 스킵)
        // =======================================================
        // (이 부분은 정밀도를 위해 메쉬 월드 좌표가 필요하므로 가볍게 구합니다)
        Vector3 w0 = (mRot * Vector3(verts[indices[i]].x * mScale.x, verts[indices[i]].y * mScale.y, verts[indices[i]].z * mScale.z)) + mPos;
        Vector3 w1 = (mRot * Vector3(verts[indices[i+1]].x * mScale.x, verts[indices[i+1]].y * mScale.y, verts[indices[i+1]].z * mScale.z)) + mPos;
        Vector3 w2 = (mRot * Vector3(verts[indices[i+2]].x * mScale.x, verts[indices[i+2]].y * mScale.y, verts[indices[i+2]].z * mScale.z)) + mPos;

        Vector3 triMin = { (std::min)({w0.x, w1.x, w2.x}), (std::min)({w0.y, w1.y, w2.y}), (std::min)({w0.z, w1.z, w2.z}) };
        Vector3 triMax = { (std::max)({w0.x, w1.x, w2.x}), (std::max)({w0.y, w1.y, w2.y}), (std::max)({w0.z, w1.z, w2.z}) };

        if (boxAABB.max.x < triMin.x || boxAABB.min.x > triMax.x) continue;
        if (boxAABB.max.y < triMin.y || boxAABB.min.y > triMax.y) continue;
        if (boxAABB.max.z < triMin.z || boxAABB.min.z > triMax.z) continue;

        // =======================================================
        // 2. [Narrow-phase] 삼각형을 박스 로컬(AABB 상태)로 역변환하여 SAT 검사
        // =======================================================
        Vector3 v0 = invBoxRot * (w0 - boxPos);
        Vector3 v1 = invBoxRot * (w1 - boxPos);
        Vector3 v2 = invBoxRot * (w2 - boxPos);

        Vector3 f0 = v1 - v0;
        Vector3 f1 = v2 - v1;
        Vector3 f2 = v0 - v2;

        // 테스트할 13개의 축 (박스가 로컬 원점 AABB이므로 박스 축은 1,0,0 / 0,1,0 / 0,0,1 로 아주 단순함)
        Vector3 axes[13] = {
            {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, // 박스 AABB의 3면
            Vector3::Cross(f0, f1),          // 삼각형의 법선 (Normal)
            Vector3::Cross({1,0,0}, f0), Vector3::Cross({1,0,0}, f1), Vector3::Cross({1,0,0}, f2),
            Vector3::Cross({0,1,0}, f0), Vector3::Cross({0,1,0}, f1), Vector3::Cross({0,1,0}, f2),
            Vector3::Cross({0,0,1}, f0), Vector3::Cross({0,0,1}, f1), Vector3::Cross({0,0,1}, f2)
        };

        float faceNormalPen = -1.0f;
        Vector3 faceLocalNormal = Vector3::Zero();
        bool isIntersecting = true;

        for (int j = 0; j < 13; ++j) {
            float sqLen = axes[j].MagnitudeSq();
            if (sqLen < 0.0001f) continue;
            Vector3 axis = axes[j] / std::sqrt(sqLen);

            // 박스(AABB) 투영 반경
            float rBox = extents.x * std::abs(axis.x) + extents.y * std::abs(axis.y) + extents.z * std::abs(axis.z);

            // 삼각형 정점 투영 (중심이 0이므로 v0, v1, v2를 직접 투영)
            float p0 = Vector3::Dot(v0, axis);
            float p1 = Vector3::Dot(v1, axis);
            float p2 = Vector3::Dot(v2, axis);
            float triMinProj = (std::min)({p0, p1, p2});
            float triMaxProj = (std::max)({p0, p1, p2});

            // 원점(Box) 투영 구간은 [-rBox, rBox]
            // 삼각형 투영 구간은 [triMinProj, triMaxProj]
            if (triMinProj > rBox || triMaxProj < -rBox) {
                isIntersecting = false; // 그림자가 안 겹침!
                break;
            }

            // 고스트 엣지 방지: 충돌 "여부"는 13축 SAT로 정확히 판정하되, 해석용 노멀/깊이는
            // 삼각형 면 법선(j==3) 기준으로 고정. 평평한 바닥을 미끄러질 때 삼각형 이음새(내부 모서리)에서
            // SAT 최소축이 옆으로 튀며 측면 임펄스+토크로 박스를 걸어 넘어뜨리는 것(텀블링 유발) 차단.
            if (j == 3) {
                faceNormalPen = (std::min)(rBox - triMinProj, triMaxProj - (-rBox));
                float triCenterProj = (p0 + p1 + p2) / 3.0f;
                faceLocalNormal = (triCenterProj < 0) ? axis : (axis * -1.0f);
            }
        }

        if (isIntersecting && faceNormalPen >= 0.0f && faceNormalPen > maxPenetration) {
            maxPenetration = faceNormalPen;
            bestWorldNormal = boxRot * faceLocalNormal;
            bestPlanePoint = w0;
            hasCollision = true;
        }
    }

    if (hasCollision) {
        Vector3 boxX = boxRot * Vector3(1, 0, 0);
        Vector3 boxY = boxRot * Vector3(0, 1, 0);
        Vector3 boxZ = boxRot * Vector3(0, 0, 1);
        Vector3 dir = bestWorldNormal * -1.0f;   // 박스 입장에서 "바닥을 향한" 방향

        // 박스 중심이 아니라, 그 방향으로 가장 튀어나온 박스의 실제 꼭짓점(서포트 포인트)을 접촉점으로 사용.
        // (예전엔 ClosestPtPointTriangle(boxPos,...)로 "박스 중심"에서 제일 가까운 삼각형 위 점을 썼는데,
        //  박스가 기울어져 있으면 실제 닿은 모서리와 다른 위치가 나와서 지렛대 팔(r)이 틀어짐 —
        //  넘어지는 중인 박스가 잘못된 토크를 받아 다 넘어가지 못하고 멈추는 원인으로 추정)
        Vector3 supportPoint = boxPos
            + boxX * (extents.x * (Vector3::Dot(boxX, dir) >= 0.0f ? 1.0f : -1.0f))
            + boxY * (extents.y * (Vector3::Dot(boxY, dir) >= 0.0f ? 1.0f : -1.0f))
            + boxZ * (extents.z * (Vector3::Dot(boxZ, dir) >= 0.0f ? 1.0f : -1.0f));

        outContact.colA = a;
        outContact.colB = b;
        outContact.rbA = GetSafeRigidbody(a);
        outContact.rbB = GetSafeRigidbody(b);
        outContact.penetration = maxPenetration;
        outContact.normal = bestWorldNormal;
        outContact.contactPoint = supportPoint;
        outContact.pointCount = 0;   // 기본은 단일접점(모서리/뾰족한 부분이 박힌 경우)

        float dotX = std::abs(Vector3::Dot(boxX, dir));
        float dotY = std::abs(Vector3::Dot(boxY, dir));
        float dotZ = std::abs(Vector3::Dot(boxZ, dir));

        // 박스의 어느 면이 접촉 평면과 거의 평행(플러시)할 때만 4점 매니폴드로 승격 —
        // 모서리 착지/기울어진 상태는 단일점이 기하학적으로 정답.
        // 0.95(약 18도)는 너무 느슨해서 아직 다 안 넘어간 상태에서도 승격돼버렸음 — 0.999(약 2.5도)로 강화.
        if ((std::max)({dotX, dotY, dotZ}) > 0.9999f) {

            Vector3 faceNormalAxis, faceU, faceV;
            float faceNormalExt, faceUExt, faceVExt;
            if (dotX >= dotY && dotX >= dotZ) {
                faceNormalAxis = boxX; faceNormalExt = extents.x;
                faceU = boxY; faceUExt = extents.y;
                faceV = boxZ; faceVExt = extents.z;
            } else if (dotY >= dotX && dotY >= dotZ) {
                faceNormalAxis = boxY; faceNormalExt = extents.y;
                faceU = boxX; faceUExt = extents.x;
                faceV = boxZ; faceVExt = extents.z;
            } else {
                faceNormalAxis = boxZ; faceNormalExt = extents.z;
                faceU = boxX; faceUExt = extents.x;
                faceV = boxY; faceVExt = extents.y;
            }
            if (Vector3::Dot(faceNormalAxis, dir) < 0.0f) faceNormalAxis = faceNormalAxis * -1.0f;

            Vector3 faceCenter = boxPos + faceNormalAxis * faceNormalExt;
            Vector3 corners[4] = {
                faceCenter + faceU * faceUExt + faceV * faceVExt,
                faceCenter + faceU * faceUExt - faceV * faceVExt,
                faceCenter - faceU * faceUExt + faceV * faceVExt,
                faceCenter - faceU * faceUExt - faceV * faceVExt,
            };

            // 승리한 삼각형이 놓인 평면에 투영 (국소적으로 평평한 바닥이라고 가정 — 일반 지형 바닥엔 타당)
            for (int i = 0; i < 4; ++i) {
                float d = Vector3::Dot(corners[i] - bestPlanePoint, bestWorldNormal);
                outContact.points[i] = corners[i] - bestWorldNormal * d;
            }
            outContact.pointCount = 4;
        }

        return true;
    }

    return false;
}


bool CollisionSolver::MeshVsMesh(Collider *a, Collider *b, Contact &outContact) {
    return false;
}
