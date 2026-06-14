//
// Created by white on 26. 1. 26..
//

#ifndef FPSPROJECTSERVER_COMPONENT_H
#define FPSPROJECTSERVER_COMPONENT_H
#include "ComponentArgument.h"
#include "ComponentManager.h"
#include "ComponentTypeCounter.h"

///Component의 직계 자식일 경우: public Component<자신>
///Component의 손자 관계일 경우: public Component<자신,직계 부모>
///추상 클래스의 경우: ComponentArgument 상속 후 그 구현체 Component<구현체,추상 클래스> 사용
template <typename T, typename Parent = ComponentArgument>
requires std::derived_from<Parent,ComponentArgument>
class Component : public Parent {
public:
    // 생성자 포워딩: 부모(Parent)의 생성자가 어떤 인자를 받든 그대로 전달
    template <typename... Args>
    explicit Component(Args&&... args) : Parent(std::forward<Args>(args)...) {
        this->typeId = GetTypeId<T>();
    }
    ///handleOut은 ComponentHandle<T>가 담겨 반환됩니다.
    void MoveToManager(ComponentManager* manager, ComponentHandleBase* handleOut) final;

    ComponentHandle<T> MakeHandle () const {
        if (this->gameSession == nullptr) LOG_ERROR("gameSession이 NULL인 Component객체가 MakeHandle을 시도함.");
        return ComponentHandle<T>(
            this->typeId,
            this->entityId,
            this->gameSession ? this->gameSession->componentManager.get() : nullptr
        );
    }

};


template<typename T, typename Parent> requires std::derived_from<Parent, ComponentArgument>
void Component<T, Parent>::
MoveToManager(ComponentManager *manager, ComponentHandleBase *handleOut) {
    std::cout << "MoveToManager" << std::endl;
    auto handle = manager->InsertOrphanageComponent<T>(static_cast<T*>(this));
    if (handleOut) {
        *handleOut = handle;
    }
}


#endif //FPSPROJECTSERVER_COMPONENT_H
