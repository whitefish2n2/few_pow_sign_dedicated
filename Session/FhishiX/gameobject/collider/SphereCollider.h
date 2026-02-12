//
// Created by white on 26. 2. 12..
//

#ifndef FPSPROJECTSERVER_CIRCLECOLLIDER_H
#define FPSPROJECTSERVER_CIRCLECOLLIDER_H


#include "Collider.h"

class SphereCollider final : public Component<SphereCollider,Collider>{
public:
    Vector3 center = Vector3::Zero();
    float height = 0;
    float radius = 0;;
    mutable bool haveMesh = false;
    SphereCollider(const bool isStatic, const Vector3& center = {0,0,0}, const float height = 1, const float radius = 1 ) : Component(isStatic), center(center), height(height), radius(radius) {}
    SphereCollider() = default;
    AABB GetAABB() const override;



    std::unique_ptr<Collider> clone() const override {
        return std::make_unique<SphereCollider>(*this);
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
};


#endif //FPSPROJECTSERVER_CIRCLECOLLIDER_H