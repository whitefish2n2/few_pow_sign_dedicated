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
    ComponentHandleBase() = default;
    ComponentHandleBase(size_t typeId, ComponentEntityId entityId):typeId(typeId),entityId(entityId){}
    virtual ~ComponentHandleBase() = default;
    ComponentEntityId entityId = -1;
    virtual ComponentHandleBase Clone() {
        return {};
    };
    static ComponentHandleBase NULLPTR() {
        return ComponentHandleBase{};
    }
    void SetFromRawHandle(ComponentEntityId entity_id, size_t type_id) {
        this->typeId = type_id;
        this->entityId = entity_id;
    }
    bool isNull() const {
        return typeId == -1 || entityId == -1;
    }
};
#endif //FPSPROJECTSERVER_COMPONENTHANDLEBASE_H