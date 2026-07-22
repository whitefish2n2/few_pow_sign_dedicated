//
// Created by white on 25. 12. 12..
//

#ifndef FPSPROJECTSERVER_RIGIDBODY_H
#define FPSPROJECTSERVER_RIGIDBODY_H
#include <iostream>

#include "../../../Component/Definition/Component.h"
#include "../../../Component/Definition/ComponentArgument.h"
#include "../../vector/Vector3.h"


class Rigidbody final:public Component<Rigidbody> {
    public:
    static constexpr int UPDATE_PRIORITY = 100;
    Rigidbody() = default;
    float mass = 1.0f;
    float inverseMass = 1.0f;
    bool useGravity = true;
    bool isKinematic = false;
    bool isDirty = false;

    Vector3 linearVelocity = Vector3(0, 0, 0);
    Vector3 angularVelocity = Vector3(0, 0, 0);   // rad/s, 월드 프레임 (Unity Rigidbody.angularVelocity 미러)
    Vector3 inertiaLocal = Vector3(0, 0, 0);         // 로컬 관성텐서 대각 합 (콜라이더들이 Start에서 합산)
    Vector3 inverseInertiaLocal = Vector3(0, 0, 0);  // 역관성. 0 = 무한관성(회전 불가) — 콜라이더 없는 바디 기본값
    Vector3 force = {0, 0, 0};
    float drag = 0.0f;
    float angularDrag = 0.0f;

    int constraints = 0;
    int collisionDetectionMode = 0;
    Vector3 centerOfMass = Vector3(0, 0, 0);

    void SetMass(float newMass) {
        mass = newMass;
        inverseMass = (mass > 0.0f) ? (1.0f / mass) : 0.0f;
        isDirty = true;
    }
    void AddForce(const Vector3& appliedForce) {
        if (isKinematic || inverseMass == 0.0f) return;

        this->force += appliedForce;
        isDirty = true;
    }




    // 인라인으로 선언해서 함수 호출 오버헤드(Call stack)를 없앱니다.
    inline void SetVelocity(const Vector3& newVelocity) {
        if (isKinematic) return;
        linearVelocity = newVelocity;
        isDirty = true;
    }
    ///Unity ForceMode.Impulse 미러: 즉시 속도 변화 Δv = impulse * inverseMass
    inline void AddImpulse(const Vector3& impulse) {
        if (isKinematic || inverseMass == 0.0f) return;
        linearVelocity += impulse * inverseMass;
        isDirty = true;
    }
    ///콜라이더가 Start에서 자기 형상 관성을 합산 (컴파운드 콜라이더 = 대각 합 근사)
    inline void AddLocalInertia(const Vector3& inertia) {
        inertiaLocal += inertia;
        inverseInertiaLocal = Vector3(
            inertiaLocal.x > 0.0001f ? 1.0f / inertiaLocal.x : 0.0f,
            inertiaLocal.y > 0.0001f ? 1.0f / inertiaLocal.y : 0.0f,
            inertiaLocal.z > 0.0001f ? 1.0f / inertiaLocal.z : 0.0f);
    }
    void FixedUpdate() override {
    };
    void PhysicsUpdate();
    void ParseFromString(const std::string &arg) override;
    void Integrate();

    void AddImpulseAtPoint(const Vector3 &worldPoint, const Vector3 &impulse);
};


#endif //FPSPROJECTSERVER_RIGIDBODY_H