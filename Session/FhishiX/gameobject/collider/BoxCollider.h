//
// Created by white on 25. 10. 28.
//

#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H
#include <memory>

#include "Collider.h"


class BoxCollider: public Collider {
    public:
    Vector3 center, size;
    mutable bool haveMesh = false;
    BoxCollider(GameObject* owner,const bool isStatic, const Vector3& center, const Vector3& size) : Collider(), center(center), size(size) {
    }
    ObjectTypeEnum GetType() const override { return ObjectTypeEnum::Box; }
    AABB GetAABB() const override {
        AABB aabb = AABB::Empty();
        return aabb;
    }
    std::unique_ptr<Collider> clone() const override {
        return std::make_unique<BoxCollider>(*this);
    }

    //BoxCollider 복사 생성자(Collider 복사 오버라이딩)
    BoxCollider(const BoxCollider& other)
        : Collider(other),
          center(other.center),
          size(other.size),
          haveMesh(other.haveMesh)
    {}
};



#endif //BOXCOLLIDER_H
