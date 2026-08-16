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
    ///identity(할당 여부)뿐 아니라 대상 컴포넌트가 지금 실제로 살아있는지(세대 일치)까지 검사합니다.
    ///정의는 ComponentManager.h에 있음(componentManager의 완전한 타입이 필요해서 아웃오브라인).
    bool isNull() const;
    bool operator==(const ComponentHandleBase& other) const {
        return entityId == other.entityId && typeId == other.typeId&& generationId == other.generationId;;
    }
};
#endif //FPSPROJECTSERVER_COMPONENTHANDLEBASE_H