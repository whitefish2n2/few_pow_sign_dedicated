//
// Created by white on 25. 5. 23.
//

#ifndef COLLIDER_H
#define COLLIDER_H

#include "../GameObjectArgument.h"
#include "../ObjectType.h"
#include "../../vector/Vector3.h"
#include "../../AABB.h"


class GameObjectArgument;
class GameObject;

struct Collision {

};

class Collider {
    public:
    GameObject* gameobject = nullptr;
    Collider(GameObject* go, const bool isStatic = false) : gameobject(go), staticObject(isStatic) {}
    Collider(const Collider& other);
    bool staticObject = false;
    virtual ~Collider() = default;
    [[nodiscard]] virtual std::unique_ptr<Collider> clone() const = 0;

    [[nodiscard]] virtual ObjectTypeEnum GetType() const = 0;
    [[nodiscard]] virtual AABB GetAABB() const = 0;
    [[nodiscard]] virtual Vector3 GetAABBSize() const {
        const auto [min, max] = GetAABB();
        return max - min;
    }
    [[nodiscard]] virtual bool AABBContainsPoint(const Vector3& point) const{
        const auto [min, max] = GetAABB();
        return ((point.x >= min.x && point.x <= max.x) && (point.y >= min.y && point.y <= max.y) && (point.z >= min.z && point.z <= max.z));
    }
    [[nodiscard]] virtual Vector3 GetAABBCenter() const{
        const auto &[min, max] = GetAABB();
        return (max + min) *0.5f;
    }
    private:
        AABB aabb = AABB::Empty();
        bool shouldUpdateAABB = true;
};



#endif //COLLIDER_H
