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
void CollisionSolver::ResolveCollision(const Contact &contact) {
    std::string msg3 = "[ResolveCollision 💥] 부딪힘! 파고든 깊이: " + std::to_string(contact.penetration) +
                       " | 튕겨낼 Normal: (" + std::to_string(contact.normal.x) + ", " +
                       std::to_string(contact.normal.y) + ", " +
                       std::to_string(contact.normal.z) + ")";
    LOG_DEBUG(msg3);
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
    if (totalInvMass <= 0.0f) return;

    const ColliderMaterial& matA = contact.colA->material;
    const ColliderMaterial& matB = contact.colB->material;

    // 반발계수 조합
    float restitution = CombineMaterial(
        matA.bounciness, matB.bounciness,
        matA.bounceCombine, matB.bounceCombine
    );

    // 마찰계수 조합 (동적)
    float friction = CombineMaterial(
        matA.dynamicFriction, matB.dynamicFriction,
        matA.frictionCombine, matB.frictionCombine
    );

    // 3. 위치 보정 (Position Resolution) - 파고든 만큼 질량비로 밀어냄
    const float SLOP = 0.01f;
    float effectivePenetration = (std::max)(contact.penetration - SLOP, 0.0f);

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

    ///충격량 계산

    Vector3 velA = rbA ? rbA->linearVelocity : Vector3::Zero();
    Vector3 velB = rbB ? rbB->linearVelocity : Vector3::Zero();
    Vector3 relativeVelocity = velA - velB;

    float velAlongNormal = Vector3::Dot(relativeVelocity, contact.normal);
    if (velAlongNormal > 0) return; // 이미 멀어지는 중

    float j = -(1.0f + restitution) * velAlongNormal;
    j /= totalInvMass;

    Vector3 normalImpulse = contact.normal * j;

    if (rbA && invMassA > 0) rbA->SetVelocity(velA + normalImpulse * invMassA);
    if (rbB && invMassB > 0) rbB->SetVelocity(velB - normalImpulse * invMassB);


    ///마찰 충격량

    // 충돌 후 갱신된 속도로 재계산
    velA = rbA ? rbA->linearVelocity : Vector3::Zero();
    velB = rbB ? rbB->linearVelocity : Vector3::Zero();
    relativeVelocity = velA - velB;

    // 접선 방향 (normal 성분 제거)
    Vector3 tangent = relativeVelocity - contact.normal * Vector3::Dot(relativeVelocity, contact.normal);
    float tangentLen = tangent.length();
    if (tangentLen < 0.0001f) return; // 접선 속도 없으면 마찰 없음
    tangent = tangent * (1.0f / tangentLen);

    float velAlongTangent = Vector3::Dot(relativeVelocity, tangent);
    float jt = -velAlongTangent / totalInvMass;

    // 쿨롱 마찰: |jt| <= friction * |j| 이면 정지 마찰, 초과하면 동적 마찰
    Vector3 frictionImpulse;
    if (std::abs(jt) <= j * friction) {
        frictionImpulse = tangent * jt;           // 정지 마찰 (완전히 멈춤)
    } else {
        frictionImpulse = tangent * (-j * friction); // 동적 마찰 (미끄러짐)
    }

    if (rbA && invMassA > 0) rbA->SetVelocity(rbA->linearVelocity + frictionImpulse * invMassA);
    if (rbB && invMassB > 0) rbB->SetVelocity(rbB->linearVelocity - frictionImpulse * invMassB);
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
            float distSq = (pts[i] - ptOnTri).LengthSquared();
            if (distSq < outMinDistSq) {
                outMinDistSq = distSq; outClosestSeg = pts[i]; outClosestTri = ptOnTri;
            }
        }

        // 2. 캡슐의 선분(Segment) vs 삼각형의 세 모서리(3 Edges)
        Vector3 edges[3][2] = { {triA, triB}, {triB, triC}, {triC, triA} };
        for (int i = 0; i < 3; ++i) {
            Vector3 c1, c2; // c1: 캡슐 선분 위의 점, c2: 삼각형 모서리 위의 점
            ClosestPtSegmentSegment(segA, segB, edges[i][0], edges[i][1], c1, c2);
            float distSq = (c1 - c2).LengthSquared();
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
        float sqLen = axis.LengthSquared();
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
bool CollisionSolver::RaycastSphere(const Ray& ray, SphereCollider* sphere, float maxDistance, RaycastHit& outHit) {
    Vector3 center = sphere->gameObject->transform.GetPosition() + sphere->center;
    Vector3 m = ray.origin - center;

    float b = Vector3::Dot(m, ray.direction);
    float c = Vector3::Dot(m, m) - (sphere->radius * sphere->radius);

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

    outHit.collider = sphere;
    outHit.distance = t;
    outHit.point = ray.origin + (ray.direction * t);
    outHit.normal = (outHit.point - center) / sphere->radius;

    return true;
}
bool CollisionSolver::RaycastBox(const Ray& ray, BoxCollider* box, float maxDistance, RaycastHit& outHit) {
    Vector3 boxPos = box->gameObject->transform.GetPosition() + box->center;
    Quaternion boxRot = box->gameObject->transform.GetRotation();
    Vector3 extents = box->size * 0.5f;

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

    Vector3 d = capBottom - capTop;
    float md = d.LengthSquared();
    if (md < EPSILON) { // 선분이 너무 짧으면 Sphere로 취급
        SphereCollider tempSphere;
        tempSphere.gameObject = cap->gameObject;
        tempSphere.radius = cap->radius;
        tempSphere.center = cap->center;
        return RaycastSphere(ray, &tempSphere, maxDistance, outHit);
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
    float c = dd * Vector3::Dot(m, m) - md_dot * md_dot - cap->radius * cap->radius * dd;

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
                    outHit.normal = (outHit.point - closestPtOnAxis) / cap->radius;
                    return true;
                }
            }
        }
    }

    // 원기둥 몸통에 안 맞았다면, 위/아래 끝단의 구(Sphere) 2개와 레이캐스트 테스트
    bool hitSphere = false;
    RaycastHit topHit, bottomHit;

    SphereCollider dummySphere;
    dummySphere.gameObject = cap->gameObject;
    dummySphere.radius = cap->radius;

    // Top Sphere
    dummySphere.center = capTop - cap->gameObject->transform.GetPosition();
    bool h1 = RaycastSphere(ray, &dummySphere, maxDistance, topHit);

    // Bottom Sphere
    dummySphere.center = capBottom - cap->gameObject->transform.GetPosition();
    bool h2 = RaycastSphere(ray, &dummySphere, maxDistance, bottomHit);

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
            float nLen = bestNormal.length();
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
    float distSq = diff.LengthSquared();
    float sumRadius = radiusA + radiusB;

    if (distSq < sumRadius * sumRadius) {
        float dist = std::sqrt(distSq);
        outContact.colA = a; outContact.colB = b;
        outContact.rbA = GetSafeRigidbody(a);
outContact.rbB = GetSafeRigidbody(b);
        outContact.penetration = sumRadius - dist;
        outContact.normal = (dist > EPSILON) ? diff / dist : Vector3(0, 1, 0);
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
    float distSq = diff.LengthSquared();
    float sumRadius = radiusA + radiusB;

    if (distSq < sumRadius * sumRadius) {
        float dist = std::sqrt(distSq);
        outContact.colA = a; outContact.colB = b;
        outContact.rbA = GetSafeRigidbody(a);
outContact.rbB = GetSafeRigidbody(b);
        outContact.penetration = sumRadius - dist;
        outContact.normal = (dist > EPSILON) ? diff / dist : Vector3(0, 1, 0);
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
    float distSq = diff.LengthSquared();
    float sumRadius = sRadius + cRadius;

    if (distSq < sumRadius * sumRadius) {
        float dist = std::sqrt(distSq);
        outContact.colA = a; outContact.colB = b;
        outContact.rbA = GetSafeRigidbody(a);
outContact.rbB = GetSafeRigidbody(b);
        outContact.penetration = sumRadius - dist;
        outContact.normal = (dist > EPSILON) ? diff / dist : Vector3(0, 1, 0);
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
        float distSq = diff.LengthSquared();

        if (distSq < radiusSq) {
            float dist = std::sqrt(distSq);
            float penetration = sRadius - dist;

            if (penetration > maxPenetration) {
                maxPenetration = penetration;
                bestNormal = (dist > EPSILON) ? (diff / dist) : Vector3(0, 1, 0);
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

        // 💡 1. 삼각형의 실제 법선(앞면 방향)을 구합니다.
        Vector3 triNormal = Vector3::Cross(v2 - v0, v1 - v0);
        float nLen = triNormal.length();
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
                // 선분이 메쉬를 완전히 뚫지는 않은 일반적인 상태
                normal = (closestSeg - closestTri) / dist;

                // 💡 2. 법선이 삼각형 뒷면을 향하고 있다면 앞면으로 강제 보정
                if (Vector3::Dot(normal, triNormal) < 0.0f) {
                    normal = normal * -1.0f;
                }
                penetration = cRadius - dist;
            } else {
                // 🚨 3. [관통 버그 해결] 선분 자체가 메쉬를 뚫어버린 심각한 상태 (dist == 0)
                normal = triNormal; // 무조건 삼각형 앞면 방향으로 밀어내기

                // 캡슐의 위(Top) 아래(Bottom) 중 바닥을 얼마나 깊이 파고들었는지 계산
                float dTop = Vector3::Dot(capTop - v0, triNormal);
                float dBot = Vector3::Dot(capBottom - v0, triNormal);

                // minD가 음수일수록 삼각형 뒷면으로 깊게 관통한 것
                float minD = (std::min)(dTop, dBot);

                // 실제 밀어내야 할 깊이 = 뚫린 깊이(-minD) + 캡슐 반지름
                penetration = cRadius - minD;
            }

            if (penetration > maxPenetration) {
                maxPenetration = penetration;
                bestNormal = normal;
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
    float distSq = localDiff.LengthSquared();
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
    float distSq = localDiff.LengthSquared();
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
        }
    }

    // 법선 방향이 항상 B에서 A를 향하도록 보정
    if (Vector3::Dot(bestAxis, centerDiff) < 0.0f) {
        bestAxis = bestAxis * -1.0f;
    }

    // 축 정규화 (TestAxis에서 정규화하기 전의 벡터가 들어왔을 수 있으므로)
    float axLen = bestAxis.length();
    if (axLen > 0.0001f) bestAxis = bestAxis / axLen;
    else bestAxis = Vector3(0, 1, 0);

    outContact.colA = a;
    outContact.colB = b;
    outContact.rbA = GetSafeRigidbody(a);
    outContact.rbB = GetSafeRigidbody(b);
    outContact.penetration = minPenetration;
    outContact.normal = bestAxis;
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
        Vector3 triWorldNormal = Vector3::Cross(w2 - w0, w1 - w0);
        float triNormLen = triWorldNormal.length();
        if (triNormLen > EPSILON) triWorldNormal = triWorldNormal / triNormLen;
        else triWorldNormal = Vector3(0, 1, 0);

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

        float minPenetration = (std::numeric_limits<float>::max)();
        Vector3 bestLocalAxis = Vector3::Zero();
        bool isIntersecting = true;

        for (int j = 0; j < 13; ++j) {
            float sqLen = axes[j].LengthSquared();
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

            // 겹친 깊이 계산
            float penetration = (std::min)(rBox - triMinProj, triMaxProj - (-rBox));
            if (penetration < minPenetration) {
                minPenetration = penetration;
                // B(Mesh) -> A(Box) 방향 보정
                float triCenterProj = (p0 + p1 + p2) / 3.0f;
                bestLocalAxis = (triCenterProj < 0) ? axis : (axis * -1.0f);
            }
        }

        if (isIntersecting && minPenetration > maxPenetration) {
            maxPenetration = minPenetration;
            bestWorldNormal = boxRot * bestLocalAxis;
            if (Vector3::Dot(bestWorldNormal, triWorldNormal) < 0.0f) {
                bestWorldNormal = bestWorldNormal * -1.0f;
            }
            hasCollision = true;
        }
    }

    if (hasCollision) {
        outContact.colA = a;
        outContact.colB = b;
        outContact.rbA = GetSafeRigidbody(a);
outContact.rbB = GetSafeRigidbody(b);
        outContact.penetration = maxPenetration;
        outContact.normal = bestWorldNormal;
        return true;
    }

    return false;
}


bool CollisionSolver::MeshVsMesh(Collider *a, Collider *b, Contact &outContact) {
    return false;
}
