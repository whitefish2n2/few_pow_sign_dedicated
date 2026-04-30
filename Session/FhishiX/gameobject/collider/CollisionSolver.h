//
// Created by white on 26. 4. 21..
//

#ifndef FPSPROJECTSERVER_COLLISIONSOLVER_H
#define FPSPROJECTSERVER_COLLISIONSOLVER_H
#include <vector>
#include <functional>

#include "Raycast.h"
#include "../../vector/Vector3.h"

class SphereCollider;
class BoxCollider;
class CapsuleCollider;
class MeshCollider;
class Collider;
class Rigidbody;

// 1. 충돌 결과 데이터 (Manifold)
struct Contact {
    Vector3 normal;       // 튕겨나갈 방향 (B에서 A를 향하는 법선 벡터)
    float penetration;    // 파고든 깊이 (이만큼 밀어내야 함)
    Vector3 contactPoint; // 실제 부딪힌 지점 (현재는 옵션)

    Collider* colA;
    Collider* colB;

    Rigidbody* rbA = nullptr;
    Rigidbody* rbB = nullptr;
};

// 2. 충돌체 타입 열거형 (기존 Collider.h에 없다면 추가해야 함)
enum class ColliderType {
    Sphere = 0,
    Box,
    Capsule,
    Mesh,
    None,
    MaxCount
};

using CollisionFunc = bool(*)(Collider* a, Collider* b, Contact& outContact);

class CollisionSolver {
private:
    // 함수 포인터를 담을 2차원 배열 (Dispatch Table)
    static CollisionFunc dispatchTable[(int)ColliderType::MaxCount][(int)ColliderType::MaxCount];

public:
    static float CombineMaterial(float a, float b, CombineMode modeA, CombineMode modeB);

    // 엔진 초기화 시점에 테이블 세팅
    static void Initialize();

    // Narrow-phase 진입점
    static bool CheckCollision(Collider* a, Collider* b, Contact& outContact);

    // 충돌 해결 (밀어내기 및 슬라이딩 연산)
    static void ResolveCollision(const Contact &contact);

    // 레이캐스팅 - 가장 먼저 맞은 콜라이더를 반환합니다.
    static bool Raycast(const Ray& ray, Collider* collider, float maxDistance, RaycastHit& outHit);

private:

    //Raycast
    static bool RaycastSphere(const Ray& ray, SphereCollider* sphere, float maxDistance, RaycastHit& outHit);
    static bool RaycastBox(const Ray& ray, BoxCollider* box, float maxDistance, RaycastHit& outHit);
    static bool RaycastCapsule(const Ray& ray, CapsuleCollider* cap, float maxDistance, RaycastHit& outHit);
    static bool RaycastMesh(const Ray& ray, MeshCollider* mesh, float maxDistance, RaycastHit& outHit);

    //충돌 네로우페이즈
    static bool SphereVsSphere(Collider* a, Collider* b, Contact &outContact);
    static bool SphereVsBox(Collider* a, Collider* b, Contact &outContact);
    static bool SphereVsCapsule(Collider* a, Collider* b, Contact &outContact);
    static bool SphereVsMesh(Collider* a, Collider* b, Contact &outContact);
    static bool BoxVsBox(Collider* a, Collider* b, Contact &outContact);
    static bool BoxVsCapsule(Collider* a, Collider* b, Contact &outContact);
    static bool BoxVsMesh(Collider* a, Collider* b, Contact &outContact);
    static bool CapsuleVsCapsule(Collider* a, Collider* b, Contact &outContact);
    static bool CapsuleVsMesh(Collider* a, Collider* b, Contact &outContact);
    static bool MeshVsMesh(Collider* a, Collider* b, Contact &outContact);
};
#endif //FPSPROJECTSERVER_COLLISIONSOLVER_H
