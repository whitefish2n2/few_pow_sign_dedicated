//
// Created by white on 25. 12. 12..
//

#ifndef FPSPROJECTSERVER_RIGIDBODY_H
#define FPSPROJECTSERVER_RIGIDBODY_H
#include <iostream>

#include "../../../Component/Definition/Component.h"
#include "../../../Component/Definition/ComponentArgument.h"
#include "../../vector/Vector3.h"


class Rigidbody:public Component<Rigidbody> {
    public:
    static constexpr int UPDATE_PRIORITY = 100;
    Rigidbody() = default;
    float mass = 1.0f;
    float inverseMass = 1.0f;
    bool useGravity = true;
    bool isKinematic = false;

    Vector3 linearVelocity = Vector3(0, 0, 0);
    Vector3 angularVelocity = Vector3(0, 0, 0);
    Vector3 force = {0, 0, 0};
    float drag = 0.0f;
    float angularDrag = 0.0f;

    int constraints;
    int collisionDetectionMode;
    Vector3 centerOfMass = Vector3(0, 0, 0);

    void SetMass(float newMass) {
        mass = newMass;
        inverseMass = (mass > 0.0f) ? (1.0f / mass) : 0.0f;
    }
    void AddForce(const Vector3& appliedForce) {
        if (isKinematic || inverseMass == 0.0f) return;

        this->force += appliedForce;
    }
    void Update() override {
        PhysicsUpdate();
    };
    void PhysicsUpdate();
    void ParseFromString(const std::string &arg) override;
    void Integrate();

};


#endif //FPSPROJECTSERVER_RIGIDBODY_H