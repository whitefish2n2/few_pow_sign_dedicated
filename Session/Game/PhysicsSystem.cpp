//
// Created by white on 25. 5. 16.
//
#include "PhysicsSystem.h"

#include "MapManager.h"
#include "../FhishiX/gameobject/GameObjectArgument.h"
#include "../FhishiX/gameobject/collider/StaticCollider.h"
#include "../FhishiX/gameobject/collider/BoxCollider.h"
#include "../FhishiX/gameobject/collider/SphereCollider.h"
#include "../FhishiX/gameobject/collider/CapsuleCollider.h"
#include "../FhishiX/gameobject/collider/MeshCollider.h"
#include "../FhishiX/gameobject/collider/SphereCollider.h"
#include "../FhishiX/gameobject/collider/StaticCollider.h"
#include "../FhishiX/gameobject/rigidBody/Rigidbody.h"
#include "../../Socket/dto/BroadcastMoveDto.h"


///MapInfo를 기반으로 target인자로 전달된 GameSession에 맵을 Construct합니다. 성공 결과를 반환합니다.
bool PhysicsSystem::Init(MapInfo map_info, GameSession *target) {
    this->mapType = map_info;
    this->session = target;
    auto constructor =  MapManager::GetInstance()->GetPhysicsMapConstructor(map_info);
    auto success =constructor->Construct(session);

    std::vector<ComponentHandle<Collider> > colliders;
    auto cmManager =  session->componentManager.get();

    session->FlushGameObject();
    cmManager->FlushComponents();

    int totalBoxes = 0, addedBoxes = 0, skipNull = 0, skipRb = 0;

    for (auto& o : *cmManager->GetOrCreatePool<BoxCollider>()) {
        // [수정1] 풀의 빈 자리(Hole)는 무시해야 합니다!
        if (!o.isActive) continue;
        totalBoxes++;

        if (!o.gameObject) {
            skipNull++;
            continue;
        }
        if (o.gameObject->hasThisComponent<Rigidbody>()) {
            skipRb++;
            continue;
        }

        // [수정2] MakeHandle() 대신 명확하게 다형성 캐스팅해서 넣습니다!
        colliders.push_back(ComponentHandle<Collider>(o.typeId, o.generationId, o.entityId, cmManager));
        addedBoxes++;
    }
    std::string reportMsg = "[TreeBuild] BoxCollider 탐색 결과 -> 총: " + std::to_string(totalBoxes) +
                            " | 트리 추가됨: " + std::to_string(addedBoxes) +
                            " | Null로 버려짐: " + std::to_string(skipNull) +
                            " | 다이나믹(RB)이라 버려짐: " + std::to_string(skipRb);
    LOG_DEBUG(reportMsg);

    for (auto& o: *cmManager->GetOrCreatePool<BoxCollider>()) {
        if (!o.gameObject || o.gameObject->hasThisComponent<Rigidbody>()) continue;
        colliders.push_back(o.MakeHandle());
    }
    for (auto& o : *cmManager->GetOrCreatePool<SphereCollider>()) {
        if (!o.gameObject || o.gameObject->hasThisComponent<Rigidbody>()) continue;
        colliders.push_back(o.MakeHandle());
    }
    for (auto& o: *cmManager->GetOrCreatePool<CapsuleCollider>()) {
        if (!o.gameObject || o.gameObject->hasThisComponent<Rigidbody>()) continue;
        colliders.push_back(o.MakeHandle());
    }
    for (auto& o: *cmManager->GetOrCreatePool<MeshCollider>()) {
        if (!o.gameObject || o.gameObject->hasThisComponent<Rigidbody>()) continue;
        colliders.push_back(o.MakeHandle());
    }

    tree.Build(colliders);

    for (auto& o : *cmManager->GetOrCreatePool<Rigidbody>()) {
        //나중에 그리드 빌드할거면 여기에
    }

    return success;
}


std::vector<Collider*> PhysicsSystem::OverlapSphere(const Vector3& center, float radius, LayerMask layerMask = LayerMask(0xFFFFFFFF)){
    std::vector<Collider*> result;

    // 1. K-D 트리에서 후보군 추출
    std::vector<ComponentHandle<Collider>> candidates;
    AABB searchBounds(center - Vector3(radius, radius, radius), center + Vector3(radius, radius, radius));
    tree.GetOverlaps(searchBounds, candidates);

    // 2. 후보군 정밀 검사
    Contact dummyContact;
    for (auto& handle : candidates) {
        Collider* col = handle.operator->();
        if (col == nullptr || !col->gameObject || !col->isActive) continue;   // 죽은 핸들/비활성 제외
        if ((layerMask & (1 << col->gameObject->layer.idx)) == 0) {
            continue;
        }
        if (CollisionSolver::OverlapSphere(center, radius, col, dummyContact)) {
            result.push_back(col);
        }
    }

    for (auto& actor : activeActors) {
        for (auto& colHandle : actor.colliders) {
            Collider* col = colHandle.operator->();
            if (col == nullptr || !col->gameObject || !col->isActive) continue;   // 죽은 핸들/비활성(픽업된 무기 등) 제외
            if ((layerMask & (1 << col->gameObject->layer.idx)) == 0) {
                continue;
            }
            if (CollisionSolver::OverlapSphere(center, radius, col, dummyContact)) {
                result.push_back(col);
            }
        }
    }

    return result;
}
bool PhysicsSystem::CheckSphere(const Vector3& center, float radius, LayerMask layerMask) {
    // 1. K-D 트리에서 AABB로 후보군 추출
    std::vector<ComponentHandle<Collider>> candidates;
    AABB searchBounds(center - Vector3(radius, radius, radius), center + Vector3(radius, radius, radius));
    tree.GetOverlaps(searchBounds, candidates);


    Contact dummyContact;

    // 2. K-D 트리 후보군 정밀 검사 (Static 객체 / 지형)
    for (auto& handle : candidates) {
        Collider* col = handle.operator->();
        if (col == nullptr || !col->gameObject || !col->isActive) continue;   // 죽은 핸들/비활성 제외

        // 레이어 마스크 체크 (아까 논의한 캐싱이 적용되었다면 col->cachedLayer.idx 로 바꾸시면 더 빠릅니다!)
        if ((layerMask & (1 << col->gameObject->layer.idx)) == 0) {
            continue;
        }

        // 💡 핵심: 하나라도 부딪히면 배열에 담을 필요 없이 즉시 true 반환! (Early Exit)
        if (CollisionSolver::OverlapSphere(center, radius, col, dummyContact)) {
            return true;
        }
    }

    // 3. 동적 객체 검사 (activeActors)
    for (auto& actor : activeActors) {
        for (auto& colHandle : actor.colliders) {
            Collider* col = colHandle.operator->();
            if (col == nullptr || !col->gameObject || !col->isActive) continue;   // 죽은 핸들/비활성(픽업된 무기 등) 제외

            if ((layerMask & (1 << col->gameObject->layer.idx)) == 0) {
                continue;
            }

            // 💡 여기서도 하나 닿으면 바로 true
            if (CollisionSolver::OverlapSphere(center, radius, col, dummyContact)) {
                return true;
            }
        }
    }

    // 끝까지 다 뒤졌는데 아무것도 안 걸렸다면
    return false;
}

bool PhysicsSystem::Raycast(const Ray &ray, float maxDistance, LayerMask layerMask, RaycastHit &outHit, const std::function<bool(Collider *)> &
                            filter) {
    // 레이 구간을 감싸는 AABB로 후보 추출 (OverlapSphere 패턴 미러)
    Vector3 end = ray.origin + ray.direction * maxDistance;
    Vector3 mn((std::min)(ray.origin.x, end.x), (std::min)(ray.origin.y, end.y), (std::min)(ray.origin.z, end.z));
    Vector3 mx((std::max)(ray.origin.x, end.x), (std::max)(ray.origin.y, end.y), (std::max)(ray.origin.z, end.z));
    std::vector<ComponentHandle<Collider>> candidates;
    tree.GetOverlaps(AABB(mn, mx), candidates);

    bool found = false;
    RaycastHit best;

    auto test = [&](Collider* col) {
        if (col == nullptr || !col->gameObject) return;
        if (!col->isActive) return;                        // 비활성(픽업된 무기 등) 제외
        if ((layerMask & (1 << col->gameObject->layer.idx)) == 0) return;
        if (filter && !filter(col)) return;                // 호출자 제외조건
        RaycastHit hit;
        if (CollisionSolver::Raycast(ray, col, maxDistance, hit) && (!found || hit.distance < best.distance)) {
            best = hit;
            found = true;
        }
    };

    for (auto& handle : candidates) test(handle.operator->());
    for (auto& actor : activeActors)
        for (auto& colHandle : actor.colliders) test(colHandle.operator->());

    if (found) outHit = best;
    return found;
}
