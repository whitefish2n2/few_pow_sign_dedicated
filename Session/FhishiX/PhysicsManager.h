//
// Created by white on 26. 3. 16..
//

#ifndef FPSPROJECTSERVER_PHYSICSMANAGER_H
#define FPSPROJECTSERVER_PHYSICSMANAGER_H
#include "../../util/util.h"
#include "../Component/Definition/Component.h"
#include "../Component/Definition/ComponentArgument.h"
#include "gameobject/rigidBody/Rigidbody.h"
class Collider;
class Rigidbody;
///Physics Loop 돌리는애->컴포넌트에 속하며 Update에 따라서 알아서 루프 돌린다.
class PhysicsManager final:public Component<PhysicsManager> {
    public:
    PhysicsManager() = default;
    ~PhysicsManager() = default;
    void ParseFromString(const std::string &arg) override {

    };
    void Update() override {
        this->PhysicsUpdate();
        //Log("PhysicsManager::PhysicsUpdate 호출해썩");
    }
    void PhysicsUpdate();
};
#endif //FPSPROJECTSERVER_PHYSICSMANAGER_H