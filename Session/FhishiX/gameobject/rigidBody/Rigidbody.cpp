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

    float dt = gameSession->time.DeltaTime;

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

    // Unity Rigidbody.maxAngularVelocity(기본 7 rad/s) 미러 — 관성 작은 물체가 충돌 임펄스로 비정상 스핀하는 것 차단
    constexpr float MAX_ANGULAR_VELOCITY = 7.0f;
    float angSq = angularVelocity.MagnitudeSq();
    if (angSq > MAX_ANGULAR_VELOCITY * MAX_ANGULAR_VELOCITY) {
        angularVelocity = angularVelocity * (MAX_ANGULAR_VELOCITY / std::sqrt(angSq));
    }

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
    // 각속도 임계값은 rad/s 기준이라 기존 0.01(=약 초당 5.7도)이 너무 커서, 넘어지는 후반부처럼
    // 토크가 작아져 자연스럽게 느려지는 정상적인 회전까지 매 틱 강제로 0으로 스냅시켜버렸음
    // (다 넘어가기 전에 각속도가 갑자기 사라지는 버그의 원인) — 진짜 미세한 잔떨림만 잡히게 낮춤.
    if (linearVelocity.MagnitudeSq() < 0.01f) linearVelocity = Vector3(0, 0, 0);
    if (angularVelocity.MagnitudeSq() < 0.0001f) angularVelocity = Vector3(0, 0, 0);

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

    // 회전 적분: dq/dt = 0.5·ω̂·q (ω: rad/s, 월드 프레임)
    // 오일러각 덧셈은 다축 회전(텀블링)에서 축이 섞여 틀림 → 쿼터니언 적분
    if (angularVelocity.MagnitudeSq() > 0.0f) {
        Quaternion q = transform->GetRotation();
        Quaternion wq(0.0f, angularVelocity.x, angularVelocity.y, angularVelocity.z);
        transform->SetRotation((q + (wq * q) * (0.5f * dt)).Normalized());
    }
}
void Rigidbody::AddImpulseAtPoint(const Vector3& worldPoint, const Vector3& impulse) {
    if (isKinematic || inverseMass == 0.0f) return;
    linearVelocity += impulse * inverseMass;                       // 기존 AddImpulse와 동일

    Quaternion rot = gameObject->transform.GetRotation();
    Vector3 r = worldPoint - (gameObject->transform.GetPosition() + rot * centerOfMass);

    if (inverseInertiaLocal.MagnitudeSq() > 0.0f) {
        Vector3 torque = Vector3::Cross(r, impulse);      // 이 충격이 만드는 회전량(월드 기준)
        Vector3 localTorque = rot.Conjugate() * torque;   // 물체 로컬 축으로 돌려서
        localTorque = Vector3(localTorque.x * inverseInertiaLocal.x,
                              localTorque.y * inverseInertiaLocal.y,
                              localTorque.z * inverseInertiaLocal.z);   // 축별 회전저항(역관성) 반영
        angularVelocity += rot * localTorque;              // 다시 월드로 되돌려 각속도에 누적
    }
    isDirty = true;
}

REGISTER_COMPONENT(Rigidbody);