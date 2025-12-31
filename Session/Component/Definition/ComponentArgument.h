//
// Created by white on 25. 11. 27.
//

#ifndef FPSPROJECTSERVER_COMPONENT_H
#define FPSPROJECTSERVER_COMPONENT_H
#include "../../FhishiX/gameobject/EntityTypes.h"
#include "../../FhishiX/gameobject/GameObject.h"
class GameSession;
class ComponentArgument {
protected:
    GameObject gameObject = GameObject::NullPTR();
    public:
    ComponentEntityId entityId = -1;
    ComponentArgument(const ComponentEntityId entityId): entityId(entityId){}
    ComponentArgument(const ComponentArgument& other);
    ComponentArgument& operator=(const ComponentArgument& other) = default;
    ComponentArgument () = default;

    virtual ~ComponentArgument () noexcept = default ;
    virtual void OnAttach() { };
    virtual void OnDetach() { };
    virtual void Update() { };
    virtual void Reset() { };
    virtual void Start() { };
    virtual void Awake(){ };

    void SetOwner(GameObject owner) {
        this->gameObject = owner;
    };
    [[nodiscard]] GameObject GetGameObject() const {
        return gameObject;
    }

};
#endif //FPSPROJECTSERVER_COMPONENT_H