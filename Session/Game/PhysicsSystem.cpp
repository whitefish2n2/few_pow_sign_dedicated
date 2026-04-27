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
#include "../../Socket/BroadcastMoveDto.h"


///MapInfo를 기반으로 target인자로 전달된 GameSession에 맵을 Construct합니다. 성공 결과를 반환합니다.
bool PhysicsSystem::Init(MapInfo map_info, GameSession *target) {
    this->mapType = map_info;
    this->session = target;
    auto constructor =  MapManager::GetInstance()->GetPhysicsMapConstructor(map_info);
    auto success =constructor->Construct(session);

    std::vector<ComponentHandle<Collider> > colliders;
    auto cmManager =  session->componentManager.get();

    session->FlushGameObject();
    session->UpdateComponents();

    // =================================================================
    // 🚨 K-D 트리에 넣을 스태틱 콜라이더 색출 작전 (디버그 로그 포함)
    // =================================================================
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
        colliders.push_back(ComponentHandle<Collider>(o.typeId, o.entityId, cmManager));
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
