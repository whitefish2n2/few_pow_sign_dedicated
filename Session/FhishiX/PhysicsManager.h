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

struct PhysicsActor {
    ComponentHandle<Rigidbody> rb = ComponentHandle<Rigidbody>::NULLPTR();
    ComponentHandle<Collider> col = ComponentHandle<Collider>::NULLPTR();

    static PhysicsActor NULLPTR() {
        return PhysicsActor();
    }

    explicit operator bool() const {
        return !rb.isNull() && !col.isNull();
    }
    bool isActive() const {
        return !rb.isNull() && !col.isNull();
    }
};
///Physics Loop 돌리는애->컴포넌트에 속하며 Update에 따라서 알아서 루프 돌린다.
class PhysicsManager final:public Component<PhysicsManager> {
private:
    std::vector<PhysicsActor> activeActors =  {PhysicsActor::NULLPTR(), };
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