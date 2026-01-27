//
// Created by white on 25. 12. 11..
//

#ifndef FPSPROJECTSERVER_COMPONENTHANDLE_H
#define FPSPROJECTSERVER_COMPONENTHANDLE_H
#include "ComponentArgument.h"
#include "ComponentHandleBase.h"
#include "ComponentTypeCounter.h"


class GameSession;
template<typename T>
class ComponentHandle: public ComponentHandleBase {
    public:
    ComponentHandle(ComponentEntityId entity_id, size_t typeId) {
        this->entityId = entity_id;
        this->typeId = typeId;
    }
    ComponentHandle() { }
    static size_t getTypeId() {
        return GetTypeId<T>();
    }

    ComponentHandleBase Clone() override;
    T *operator->();


    T& operator*() { return *operator->(); }

    const T& operator*() const { return *operator->(); }

    static ComponentHandle<T> NULLPTR() {
        return ComponentHandle(static_cast<ComponentEntityId>(-1),GetTypeId<T>());;
    }
};
#endif //FPSPROJECTSERVER_COMPONENTHANDLE_H
