//
// Created by white on 25. 12. 12..
//

#include "Rigidbody.h"

#include "../../FhishiX.h"
#include "../../../../util/StringUtil.h"
#include "../../../Component/Definition/ComponentFactory.h"


// Integrate로 이관
/*
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
*/

void Rigidbody::ParseFromString(const std::string& arg) {
    std::stringstream ss(arg);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        size_t delimPos = line.find(':');
        if (delimPos == std::string::npos) continue;
        std::string key = StringUtils::Trim(line.substr(0, delimPos));
        std::string val = StringUtils::Trim(line.substr(delimPos + 1));

        if (key == "Mass") {
            this->SetMass(std::stof(val));
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
    // 안전 장치 추가
    if (!gameObject) {
        LOG_ERROR("Integrate에서 GameObject가 없는 RigidBody 객체가 업데이트를 시도함.");
        return;
    }

    if (isKinematic || inverseMass <= 0.0f) return;

    float dt = gameSession->time.FixedDeltaTime;

    /*
    std::string msg2 = "[Integrate 시작] dt: " + std::to_string(dt) +
                       " | mass: " + std::to_string(mass) +
                       " | useGravity: " + std::to_string(useGravity) +
                       " | 현재 Vel.y: " + std::to_string(linearVelocity.y) +
                       " | 현재 Pos.y: " + std::to_string(gameObject->transform.GetPosition().y);
    LOG_DEBUG(msg2);
    */
    // ----------------------------------------------------
    // 1. 힘(Force) 적용 및 속도(Velocity) 갱신
    // ----------------------------------------------------
    if (useGravity) {
        force.y -= mass * Gravity;
    }

    Vector3 acceleration = force * inverseMass;

    linearVelocity = linearVelocity + (acceleration * dt);

    linearVelocity = linearVelocity * (std::max)(0.0f, 1.0f - (drag * dt));
    angularVelocity = angularVelocity * (std::max)(0.0f, 1.0f - (angularDrag * dt));

    // ----------------------------------------------------
    // 2. 제약 조건(Constraints) 적용
    // ----------------------------------------------------
    if (constraints & 2) linearVelocity.x = 0.0f;
    if (constraints & 4) linearVelocity.y = 0.0f;
    if (constraints & 8) linearVelocity.z = 0.0f;

    if (constraints & 16) angularVelocity.x = 0.0f;
    if (constraints & 32) angularVelocity.y = 0.0f;
    if (constraints & 64) angularVelocity.z = 0.0f;

    ///미세 떨림 방지
    if (linearVelocity.LengthSquared() < 0.0001f) linearVelocity = Vector3(0, 0, 0);
    if (angularVelocity.LengthSquared() < 0.0001f) angularVelocity = Vector3(0, 0, 0);

    // ----------------------------------------------------
    // 3. 누적된 힘(Force) 초기화
    // ----------------------------------------------------
    force = Vector3(0, 0, 0);

    // ----------------------------------------------------
    // 4. 위치(Position) 및 회전(Rotation) 갱신 (일단 움직인다!)
    // ----------------------------------------------------
    Transform* transform = &gameObject->transform;

    Vector3 newPos = transform->GetPosition() + (linearVelocity * dt);
    transform->SetPosition(newPos);

    Vector3 newEuler = transform->GetEularRotation() + (angularVelocity * dt);
    transform->SetRotation(Quaternion::FromEuler(newEuler));
}


REGISTER_COMPONENT(Rigidbody);