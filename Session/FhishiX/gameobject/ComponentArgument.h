//
// Created by white on 25. 11. 27.
//

#ifndef FPSPROJECTSERVER_COMPONENT_H
#define FPSPROJECTSERVER_COMPONENT_H
#include <ranges>

#include "ComponentManager.h"
#include "ComponentManager.h"
#include "../../GameSession.h"

class GameObject;
class GameSession;
class ComponentArgument {
protected:
    GameObject* gameObject = nullptr;
    GameSession* session = nullptr;
    public:
    ComponentEntityId entityId;
    ComponentArgument(const ComponentEntityId entityId, GameSession* session):session(session), entityId(entityId){}
    virtual ~ComponentArgument () = default ;
    virtual void OnAttach() = 0;
    virtual void OnDetach() = 0;
    virtual void Update() = 0;
    virtual void Reset() = 0;
    virtual void Start() = 0;
    virtual void Awake() = 0;
    void SetOwner(GameObject *owner) {
        this->gameObject = owner;
    };
    [[nodiscard]] GameObject* GetGameObject() const {
        return gameObject;
    }

};
#endif //FPSPROJECTSERVER_COMPONENT_H