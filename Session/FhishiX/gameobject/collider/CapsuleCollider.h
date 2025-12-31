//
// Created by white on 25. 11. 14..
//

#ifndef FPSPROJECTSERVER_CAPSULECOLLIDER_H
#define FPSPROJECTSERVER_CAPSULECOLLIDER_H
#include <minwindef.h>

#include "Collider.h"

class CapsuleCollider : public Collider {
public:
    Vector3 center;
    float height, radius;
    mutable bool haveMesh = false;
    explicit CapsuleCollider(const bool isStatic, const Vector3& center = {0,0,0}, const float height = 1, const float radius = 1 ) : Collider(isStatic), center(center), height(height), radius(radius) {}
    ObjectTypeEnum GetType() const override { return ObjectTypeEnum::Capsule; }
    AABB GetAABB() const override;



    //CapsuleCollider 복사 생성자(Collider 복사 오버라이딩)
    CapsuleCollider(const CapsuleCollider& other):
        Collider(other.staticObject),
        center(other.center),
        height(other.height),
        radius(other.radius),
        haveMesh(other.haveMesh)
    {}

    std::unique_ptr<Collider> clone() const override {
        return std::make_unique<CapsuleCollider>(*this);
    }

    Vector3 GetAABBSize() const override {
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
};
#endif //FPSPROJECTSERVER_CAPSULECOLLIDER_H