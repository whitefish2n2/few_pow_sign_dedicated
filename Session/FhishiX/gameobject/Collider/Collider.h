//
// Created by white on 25. 5. 23.
//

#ifndef COLLIDER_H
#define COLLIDER_H

#include "../ObjectType.h"
#include "../../Vector/Vector3.h"
#include "../../AABB.h"



class GameObject;

struct Collision {

};

class Collider {
    protected:
    // 자신이 속한 객체를 가리키는 포인터
    GameObject* gameobject = nullptr;
    public:
    Collider(GameObject* go, const bool isStatic = false) : gameobject(go), staticObject(isStatic) {}
    Collider(const Collider& other);
    bool staticObject = false;
    virtual ~Collider() = default;
    virtual std::unique_ptr<Collider> clone() const = 0;

    virtual ObjectTypeEnum GetType() const = 0;
    virtual AABB GetAABB() const = 0;
    virtual Vector3 GetAABBSize() const {
        const AABB aabb = GetAABB();
        return aabb.max - aabb.min;
    }
    virtual bool AABBContainsPoint(const Vector3& point) const{
        auto aabb = GetAABB();
        return ((point.x >= aabb.min.x && point.x <= aabb.max.x) && (point.y >= aabb.min.y && point.y <= aabb.max.y) && (point.z >= aabb.min.z && point.z <= aabb.max.z));
    }
    virtual Vector3 GetAABBCenter() const{
        const auto &[min, max] = GetAABB();
        return (max + min) *0.5f;
    }
    private:
        AABB aabb = AABB::Empty();
        bool shouldUpdateAABB = true;
};



#endif //COLLIDER_H
