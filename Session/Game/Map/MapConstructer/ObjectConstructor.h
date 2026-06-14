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

    GameObject Construct(GameSession *gameSession) const;

    std::string name;
    Layer layer = Layer(0);
    Transform transform = Transform();
    Tag tag;
    std::vector<ComponentConstructor> components = std::vector<ComponentConstructor>();

    static ObjectConstructor NullPTR() {
        return {};
    }
};


#endif //FPSPROJECTSERVER_OBJECTCONSTRUCTER_H