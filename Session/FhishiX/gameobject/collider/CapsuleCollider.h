//
// Created by white on 25. 11. 14..
//

#ifndef FPSPROJECTSERVER_CAPSULECOLLIDER_H
#define FPSPROJECTSERVER_CAPSULECOLLIDER_H

#include "Collider.h"
#include "../../../Component/Definition/Component.h"

class CapsuleCollider final : public Component<CapsuleCollider,Collider>{
public:
    Vector3 center = Vector3::Zero();
    float height = 0;
    float radius = 0;
    int direction;;//0:X,1:Y,2:Z
    mutable bool haveMesh = false;
    CapsuleCollider(const bool isStatic, const Vector3& center = {0,0,0}, const float height = 1, const float radius = 1 ) : Component(ColliderType::Capsule, isStatic), center(center), height(height), radius(radius) {}
    CapsuleCollider() : Component(ColliderType::Capsule, false) {}
    AABB GetAABB() const override;
    Vector3 CalculateLocalInertia(float mass) const override;


    std::unique_ptr<Collider> clone() const override {
        return std::make_unique<CapsuleCollider>(*this);
    }

    Vector3 GetAABBSize() const override {
        return Vector3::Zero();
    }

    bool ContainsPoint(const Vector3& point) const override {
        // 로직 비움
        return false;
    }

    bool IntersectsAABB(const GameObjectArgument& other) const override {
        // 로직 비움
        return false;
    }

    void ExpandAABB(const Vector3& point) const override {
        // 로직 비움
    }

    void MergeAABB(const AABB& other) const override {
        // 로직 비움
    }

    void CalculateAABB() const override {

    }
    void ParseFromString(const std::string &arg) override;

#ifdef _WIN64
    Renderer GetRenderer() override;
#endif
};
#endif //FPSPROJECTSERVER_CAPSULECOLLIDER_H