//
// Created by white on 26. 1. 26..
//

#ifndef FPSPROJECTSERVER_OBJECTCONSTRUCTER_H
#define FPSPROJECTSERVER_OBJECTCONSTRUCTER_H

#include <string>

#include "ComponentConstructor.h"
#include "../../../FhishiX/Layer.h"
#include "../../../FhishiX/TagManager.h"
#include "../../../FhishiX/gameobject/GameObject.h"

class ObjectConstructor {
    public:
    ObjectConstructor() =default;
    ~ObjectConstructor() =default;
    std::string name;
    Layer layer;
    Transform transform;
    Tag tag;
    std::vector<ComponentConstructor> components;
    GameObject Construct(GameSession* gameSession);
};


#endif //FPSPROJECTSERVER_OBJECTCONSTRUCTER_H