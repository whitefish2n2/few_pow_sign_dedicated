//
// Created by white on 25. 5. 15.
//

#pragma once

#include "KDTree.h"
#include "../Dto/MapInfo.h"
#include "../FhishiX/Layer.h"
#include "../FhishiX/gameobject/GameObject.h"

class Collider;

struct PhysicsActor {
    ComponentHandle<Rigidbody> rb = ComponentHandle<Rigidbody>::NULLPTR();
    std::vector<ComponentHandle<Collider>> colliders;

    explicit operator bool() const {
        return this->isActive();
    }
    static PhysicsActor NULLPTR() {
        return PhysicsActor();
    }
    bool isActive() const {
        return !rb.isNull() && !colliders.empty();
    }
};
//loadmap으로 맵 버텍스,트라이앵글 정보를 불러와요
//init으로 맵의 진행상황, 트리거같은걸 초기화해요
//Physics 관련한 정보(Static Collider, 동적객체 그리드,레이어 등) 저장하고 관리하는 객체
class PhysicsSystem
{
    public:
    MapInfo mapType;
    LayerManager layerManager;
    GameSession* session;
    KDTree tree = KDTree();
    std::vector<PhysicsActor> activeActors = { PhysicsActor::NULLPTR(), };
    void SetLayerManager(LayerManager&& layerManager) {
        this->layerManager = layerManager;
    }


    bool Init(MapInfo map_info, GameSession *target);

    std::vector<Collider *> OverlapSphere(const Vector3 &center, float radius, LayerMask layerMask);

    bool CheckSphere(const Vector3 &center, float radius, LayerMask layerMask);


    bool Raycast(const Ray &ray, float maxDistance, LayerMask layerMask, RaycastHit &outHit,
                 const std::function<bool(Collider*)> &filter = nullptr);

    PhysicsSystem() = default;
    ~PhysicsSystem() = default;
    PhysicsSystem(const PhysicsSystem &) = delete;
    PhysicsSystem(PhysicsSystem &&) noexcept = delete;

    PhysicsSystem &operator=(const PhysicsSystem& target) {
        if (this == &target) return *this;

        mapType = target.mapType;

        return *this;
    }
};

