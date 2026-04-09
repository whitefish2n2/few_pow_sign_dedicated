//
// Created by white on 26. 3. 16..
//

#ifndef FPSPROJECTSERVER_PHYSICSMANAGER_H
#define FPSPROJECTSERVER_PHYSICSMANAGER_H
#include "../../util/util.h"
#include "../Component/Definition/Component.h"
#include "../Component/Definition/ComponentArgument.h"

class PhysicsManager:public Component<PhysicsManager> {
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