//
// Created by white on 26. 1. 27..
//

#ifndef FPSPROJECTSERVER_MAPCONSTRUCTOR_H
#define FPSPROJECTSERVER_MAPCONSTRUCTOR_H
#include <iostream>

#include "ObjectConstructor.h"
#include "PhysicsSystem.h"
#include "../../../Dto/MapInfo.h"

class PhysicsSystemConstructor {
    public:
    MapInfo mapInfo;
    LayerManager layerManager;

    ///target에 Constructor들을 GameObject및 Component로 변환하여 target의 GameObjectManager와 ComponentManager에 저장합니다. 성공 여부를 반환합니다.
    [[nodiscard]] bool Construct(GameSession* target) {
        //std::cout<<"construct start!!!"<< std::endl;
        //std::cout << "Target Address: " << target << std::endl;
        target->physicsSystem->layerManager = layerManager;
        try {
            for (const auto& o: objects) {
                o.Construct(target);
                //std::cout<<o.name<<" object 생성"<< std::endl;
            }
            return true;
        }
        catch (std::exception& e) {
            std::cout << "Map Construct Failed. Map Id:"<<mapInfo.GetName()<<"error:"<< e.what() << std::endl;
            return false;
        }

    }
    void SetLayerManager(LayerManager&& lm) {
        layerManager = std::move(lm);

    }
    void InsertObject(std::unique_ptr<ObjectConstructor> o) {
        objects.push_back(std::move(*o));
    }
    std::vector<ObjectConstructor> objects;

};


#endif //FPSPROJECTSERVER_MAPCONSTRUCTOR_H