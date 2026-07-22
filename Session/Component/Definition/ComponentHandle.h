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
class ComponentHandle final: public ComponentHandleBase {
    public:

    using ComponentHandleBase::ComponentHandleBase;
    ComponentHandle() = default;

    [[nodiscard]] size_t getTypeId() const {
        return typeId;
    }

    ComponentHandleBase Clone() override;
    T *operator->() const;


    T& operator*() { return *operator->(); }

    const T& operator*() const { return *operator->(); }

    static ComponentHandle<T> NULLPTR() {
        return ComponentHandle(GetTypeId<T>(),-1, static_cast<ComponentEntityId>(-1));;
    }

    ///핸들을 추상화 시켜주는 함수->ComponentHandle<부모>(자식핸들) 형태로 사용, 타입ID는 보존됨으로 성능엔 영향없음...
    template <typename U>
    requires std::derived_from<U, T>
    ComponentHandle(const ComponentHandle<U>& childHandle) {
        // 자식 핸들의 데이터를 그대로 복사해서 부모 핸들을 만듭니다.
        this->typeId = childHandle.typeId;
        this->entityId = childHandle.entityId;
        this->generationId = childHandle.generationId;
        this->componentManager = childHandle.componentManager;
    }
};
#endif //FPSPROJECTSERVER_COMPONENTHANDLE_H
