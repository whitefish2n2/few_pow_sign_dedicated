//
// Created by white on 26. 3. 16..
//
#include "PhysicsManager.h"

#include <vector>

#include "../GameSession.h"
#include "../Component/Definition/ComponentFactory.h"
#include "../Component/Definition/ComponentManager.h"
#include "gameobject/rigidBody/Rigidbody.h"
REGISTER_COMPONENT(PhysicsManager);
struct DynamicProxy {
    AABB bounds;
    Collider* original;
    Rigidbody* rb;
    ComponentHandle<Collider> collider;
};
void PhysicsManager::PhysicsUpdate() {
    DrivenPool<Rigidbody>* rigidBodies = gameSession->componentManager.get()->GetOrCreatePool<Rigidbody>();

    for (auto& v : *rigidBodies) {
        if (v.isActive && v.gameObject) {
            v.Integrate();
        }
    }
    {

    }
    ///충돌처리

    for (auto& v : *rigidBodies) {
        if (v.isActive && v.gameObject)
            v.PhysicsUpdate();
        ///힘초기화
    }
}



