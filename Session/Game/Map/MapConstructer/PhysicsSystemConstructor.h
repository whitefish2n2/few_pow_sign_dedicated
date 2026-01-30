//
// Created by white on 26. 1. 27..
//

#ifndef FPSPROJECTSERVER_MAPCONSTRUCTOR_H
#define FPSPROJECTSERVER_MAPCONSTRUCTOR_H
#include "ObjectConstructor.h"
#include "../../../Dto/MapInfo.h"


class PhysicsSystemConstructor {
    public:
    MapInfo mapInfo;
    LayerManager layerManager;

    void Construct(GameSession* target) {
        for (auto o: objects) {
            o.Construct(target);
        }
    }
    void SetLayerManager(LayerManager&& lm) {
        layerManager = std::move(lm);
    }
    void InsertObject(std::unique_ptr<ObjectConstructor> o) {
        objects.push_back(std::move(*o));
    }
    private:
    std::vector<ObjectConstructor> objects;
};


#endif //FPSPROJECTSERVER_MAPCONSTRUCTOR_H