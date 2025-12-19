//
// Created by white on 25. 11. 27.
//

#ifndef FPSPROJECTSERVER_COMPONENT_H
#define FPSPROJECTSERVER_COMPONENT_H

#include "ComponentManager.h"
#include "../../GameSession.h"

class GameObject;
class GameSession;
class ComponentArgument {
protected:
    GameObject gameObject = GameObject::NullPTR();
    public:
    ComponentEntityId entityId = -1;
    ComponentArgument(const ComponentEntityId entityId): entityId(entityId){}
    ComponentArgument(const ComponentArgument& other):gameObject(GameObject::NullPTR()), entityId(-1) {
        //entityId와 gameObject는 복사 금지
    }
    ComponentArgument& operator=(const ComponentArgument& other) = default;
    ComponentArgument();

    virtual ~ComponentArgument () = default ;
    virtual void OnAttach() = 0;
    virtual void OnDetach() = 0;
    virtual void Update() = 0;
    virtual void Reset() = 0;
    virtual void Start() = 0;
    virtual void Awake() = 0;

    void SetOwner(GameObject owner) {
        this->gameObject = owner;
    };
    [[nodiscard]] GameObject GetGameObject() const {
        return gameObject;
    }

};
#endif //FPSPROJECTSERVER_COMPONENT_H