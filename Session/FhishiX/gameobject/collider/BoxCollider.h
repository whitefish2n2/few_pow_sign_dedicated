#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H
#include <memory>

#include "Collider.h"

class BoxCollider : public Collider {
public:
    Vector3 center, size;
    mutable bool haveMesh = false;

    BoxCollider(const bool isStatic, const Vector3& center, const Vector3& size)
        : Collider(isStatic), center(center), size(size) {
    }

    // 복사 생성자
    BoxCollider(const BoxCollider& other): Collider(other),
          center(other.center),
          size(other.size),
          haveMesh( other.haveMesh)
    {}

    std::unique_ptr<Collider> clone() const override {
        return std::make_unique<BoxCollider>(*this);
    }

    ObjectTypeEnum GetType() const override {
        return ObjectTypeEnum::Box;
    }

    AABB GetAABB() const override {
        // 더미 AABB
        return AABB::Empty();
    }

    Vector3 GetAABBSize() const override {
        // 박스 기준으로 그냥 size 반환
        return size;
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

#endif // BOXCOLLIDER_H
