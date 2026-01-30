//
// Created by white on 25. 5. 15.
//

#pragma once
#include <unordered_map>

#include "KDTree.h"
#include "../Dto/MapInfo.h"
#include "../FhishiX/Layer.h"
#include "../FhishiX/gameobject/GameObject.h"
//loadmap으로 맵 버텍스,트라이앵글 정보를 불러와요
//init으로 맵의 진행상황, 트리거같은걸 초기화해요

class PhysicsSystem
{
    public:
    MapInfo type;
    LayerManager layerManager;
    void SetLayerManager(LayerManager&& layerManager) {
        this->layerManager = layerManager;
    }
    std::unordered_map<uint32_t, GameObject> objects;
    std::unordered_map<uint32_t,GameObject> MovableObjects;
    std::unordered_map<uint32_t,GameObject> PlayerObjects;


    void Init();
    PhysicsSystem() = default;
    PhysicsSystem(MapInfo type) {
        this->type = type;
        //todo: 맵 타입에 대해 MapManager에서 constructer를 가져와서 여기에다가 생성.
        //todo: 맵 타입에 대해 MapManager에서 staticPhysicsMap 포인터를 가져온다.
    }
    ~PhysicsSystem();
    PhysicsSystem(const PhysicsSystem &);
    PhysicsSystem(PhysicsSystem &&) noexcept;

    PhysicsSystem &operator=(const PhysicsSystem& target) {
        if (this == &target) return *this;

        type = target.type;
        objects = target.objects;
        MovableObjects = target.MovableObjects;
        PlayerObjects = target.PlayerObjects;

        return *this;
    }
};

