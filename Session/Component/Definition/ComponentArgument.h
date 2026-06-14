//
// Created by white on 25. 11. 27.
//

#ifndef FPSPROJECTSERVER_COMPONENT_ARGUMENT_H
#define FPSPROJECTSERVER_COMPONENT_ARGUMENT_H
#include <string>

#include "ComponentHandle.h"
#include "ComponentHandleBase.h"
#include "../../FhishiX/gameobject/EntityTypes.h"
#include "../../FhishiX/gameobject/GameObject.h"

class ComponentManager;
class GameSession;
class ComponentArgument {
protected:
    public:

    /// Update 우선순위(자식에서 해당 변수 재선언으로 값 변경 가능. 큰 양수일수록 먼저 실행됨)
    static constexpr int UPDATE_PRIORITY = 0;

    GameObject gameObject = GameObject::NullPTR();
    ComponentEntityId entityId = -1;
    size_t typeId;
    bool isActive = true;
    bool willDead = false;
    static constexpr bool DO_UPDATE = true;
    GameSession* gameSession = nullptr;
    ComponentArgument(const ComponentEntityId entityId): entityId(entityId){}
    ComponentArgument(ComponentArgument&& other) = default;
    ComponentArgument(const ComponentArgument& other) = default;
    ComponentArgument& operator=(const ComponentArgument& other) = default;
    ComponentArgument& operator=(ComponentArgument&& other) = default;
    ComponentArgument () = default;

    virtual ~ComponentArgument () noexcept = default ;
protected:
    virtual void OnAttach() { };
    virtual void OnDetach() { };

public:
    virtual void Update() { };
    virtual void FixedUpdate(){ };
    virtual void Reset() { };
    virtual void Start() { };
    virtual void Awake(){ };
    virtual void OnDestroy() { };
    ///String형식으로 된 컴포넌트 정보를 토대로 컴포넌트를 초기화하는 함수(컴포넌트 ID 등은 이전되지 않음)
    virtual void ParseFromString(const std::string& arg) { };

    void SetOwner(const GameObject &owner) {
        this->gameObject = owner;
        OnAttach();
    };
    [[nodiscard]] GameObject GetGameObject() const {
        return gameObject;
    }
    ///컴포넌트 매니저에 해당 컴포넌트를 편입하는 함수
    virtual void MoveToManager(ComponentManager* manager, ComponentHandleBase* handleOut) { };
};
#endif