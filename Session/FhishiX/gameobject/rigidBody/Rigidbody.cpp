//
// Created by white on 25. 12. 12..
//

#include "Rigidbody.h"

#include "../../FhishiX.h"
#include "../../../Component/Definition/ComponentFactory.h"



void Rigidbody::PhysicsUpdate() {
    if (!gameObject) {
        LOG_ERROR("PhysicsUpdate에서 GameObject가 없는 RigidBody 객체가 업데이트를 시도함.");
        return;
    }
    if (isKinematic) return;


    float dt = gameSession->time.FixedDeltaTime;

    if (constraints & 2) linearVelocity.x = 0.0f;
    if (constraints & 4) linearVelocity.y = 0.0f;
    if (constraints & 8) linearVelocity.z = 0.0f;

    if (constraints & 16) angularVelocity.x = 0.0f;
    if (constraints & 32) angularVelocity.y = 0.0f;
    if (constraints & 64) angularVelocity.z = 0.0f;

    Transform* transform = &gameObject->transform;

    Vector3 newPos = transform->GetPosition() + (linearVelocity * dt);
    transform->SetPosition(newPos);

    Vector3 newEuler = transform->GetEularRotation() + (angularVelocity * dt);
    transform->SetRotation(Quaternion::FromEuler(newEuler));
}

void Rigidbody::ParseFromString(const std::string& arg) {
    std::stringstream ss(arg);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        size_t delimPos = line.find(": ");
        if (delimPos == std::string::npos) continue;

        std::string key = line.substr(0, delimPos);
        std::string val = line.substr(delimPos + 2);

        if (key == "Mass") {
            this->mass = std::stof(val);
            this->inverseMass = 1.0f / this->mass;
        }
        else if (key == "Drag") {
            this->drag = std::stof(val);
        }
        else if (key == "AngularDrag") {
            this->angularDrag = std::stof(val);
        }
        else if (key == "UseGravity") {
            this->useGravity = (val == "1");
        }
        else if (key == "IsKinematic") {
            this->isKinematic = (val == "1");
        }
        else if (key == "Constraints") {
            this->constraints = std::stoi(val);
        }
        else if (key == "CollisionDetection") {
            this->collisionDetectionMode = std::stoi(val);
        }
        else if (key == "CenterOfMass") {
            this->centerOfMass = Vector3::ParseVector3(val);
        }
    }
}
void Rigidbody::Integrate() {
    if (isKinematic || inverseMass <= 0.0f) return;

    float dt = gameSession->time.FixedDeltaTime;

    if (useGravity) {
        force.y -= mass * Gravity;
    }

    Vector3 acceleration = force * inverseMass;
    linearVelocity = linearVelocity + (acceleration * dt);

    float linearDragFactor = 1.0f - (drag * dt);
    if (linearDragFactor < 0.0f) linearDragFactor = 0.0f;
    linearVelocity = linearVelocity * linearDragFactor;

    float angularDragFactor = 1.0f - (angularDrag * dt);
    if (angularDragFactor < 0.0f) angularDragFactor = 0.0f;
    angularVelocity = angularVelocity * angularDragFactor;

    force = Vector3(0, 0, 0);
}


REGISTER_COMPONENT(Rigidbody);