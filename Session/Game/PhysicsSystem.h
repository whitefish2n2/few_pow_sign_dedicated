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
//미사용, 추후 관심사 분리하면 여기 쓸듯->현재는 물리 로직 GameSession이 담당
//Physics 관련한 정보(Static Collider, 동적객체 그리드,레이어 등) 저장하고 관리하는 객체
class PhysicsSystem
{
    public:
    MapInfo type;
    LayerManager layerManager;
    void SetLayerManager(LayerManager&& layerManager) {
        this->layerManager = layerManager;
    }


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

        return *this;
    }
};

