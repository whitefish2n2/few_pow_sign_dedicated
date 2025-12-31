//
// Created by white on 25. 12. 15.
//

#ifndef FPSPROJECTSERVER_COMPONENTHANDLEBASE_H
#define FPSPROJECTSERVER_COMPONENTHANDLEBASE_H
#include "ComponentTypeCounter.h"
#include "../../FhishiX/gameobject/EntityTypes.h"
class GameSession;

class ComponentHandleBase {
    public:
    size_t typeId = -1;
    ComponentHandleBase();
    virtual ~ComponentHandleBase() = default;
    ComponentEntityId entityId = -1;
    virtual ComponentHandleBase Clone() {
        return {};
    };
};
#endif //FPSPROJECTSERVER_COMPONENTHANDLEBASE_H