//
// Created by white on 25. 5. 15.
//

#pragma once
#include <unordered_map>

#include "BspTree.h"
#include "../Dto/MapEnum.h"
#include "../FhishiX/gameobject/GameObject.h"
//loadmap으로 맵 버텍스,트라이앵글 정보를 불러와요
//init으로 맵의 진행상황, 트리거같은걸 초기화해요

class Map
{
    public:
    MapEnum type;

    std::unordered_map<std::string, GameObject> objects;
    std::unordered_map<std::string,GameObject> MovableObjects;
    std::unordered_map<std::string,GameObject> PlayerObjects;

    BSPTree staticObjectsBSP;

    void Init();
    Map() : type(MapEnum::test) {}
    Map(MapEnum type);
    ~Map();
    Map(const Map &);
    Map(Map &&);

    Map &operator=(const Map & target) {
        if (this == &target) return *this;

        type = target.type;
        objects = target.objects;
        MovableObjects = target.MovableObjects;
        PlayerObjects = target.PlayerObjects;
        staticObjectsBSP = target.staticObjectsBSP;

        return *this;
    }
};

