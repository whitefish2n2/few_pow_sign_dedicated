//
// Created by white on 25. 10. 28.
//

#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H
#include <memory>

#include "Collider.h"
#include "../GameObject.h"


class BoxCollider: public Collider {
    public:
    Vector3 center, size;
    mutable bool haveMesh = false;
    BoxCollider(GameObject* owner,const bool isStatic, const Vector3& center, const Vector3& size) : Collider(owner,isStatic), center(center), size(size) {
    }
    ObjectTypeEnum GetType() const override { return ObjectTypeEnum::Box; }
    AABB GetAABB() const override {
        AABB aabb = AABB::Empty();
        return aabb;
    }
    Vector3 GetAABBSize() const override {
        return Vector3::Zero();
        //todo ㅇ
    }
    Vector3 GetAABBCenter() const override {
        const auto &abTemp = GetAABB();
        return (abTemp.max + abTemp.min) / 2.0f;
    }
    bool AABBContainsPoint(const Vector3& point) const override {
        auto aabb = GetAABB();
        return ((point.x >= aabb.min.x && point.x <= aabb.max.x) && (point.y >= aabb.min.y && point.y <= aabb.max.y) && (point.z >= aabb.min.z && point.z <= aabb.max.z));
    }
    std::unique_ptr<Collider> clone() const override {
        return std::make_unique<BoxCollider>(*this);
    }

    //BoxCollider 복사 생성자(Collider 복사 오버라이딩)
    BoxCollider(const BoxCollider& other)
        : Collider(other.gameobject, other.staticObject),
          center(other.center),
          size(other.size),
          haveMesh(other.haveMesh)
    {}
};



#endif //BOXCOLLIDER_H
