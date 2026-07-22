//
// Created by white on 25. 12. 15.
//

#ifndef FPSPROJECTSERVER_COMPONENTHANDLEBASE_H
#define FPSPROJECTSERVER_COMPONENTHANDLEBASE_H
#include "../../FhishiX/gameobject/EntityTypes.h"

class ComponentManager;

class ComponentHandleBase {
    public:
    size_t typeId = -1;
    ComponentGenerationId generationId = -1;
    ComponentManager* componentManager = nullptr;
    ComponentHandleBase() = default;
    ComponentHandleBase(size_t typeId, ComponentGenerationId generationId, ComponentEntityId entityId, ComponentManager* componentManager = nullptr):typeId(typeId),generationId(generationId), entityId(entityId),componentManager(componentManager){}
    virtual ~ComponentHandleBase() = default;
    ComponentEntityId entityId = -1;
    virtual ComponentHandleBase Clone() {
        return {};
    };
    static ComponentHandleBase NULLPTR() {
        return ComponentHandleBase{};
    }
    void SetFromRawHandle(ComponentEntityId entity_id, size_t type_id, ComponentGenerationId generation_key) {
        this->typeId = type_id;
        this->entityId = entity_id;
        this->generationId = generation_key;
    }
    bool isNull() const {
        return typeId == -1 || entityId == -1;
    }
    bool operator==(const ComponentHandleBase& other) const {
        return entityId == other.entityId && typeId == other.typeId&& generationId == other.generationId;;
    }
};
#endif //FPSPROJECTSERVER_COMPONENTHANDLEBASE_H