//
// Created by white on 25. 5. 23.
//

#ifndef COLLIDER_H
#define COLLIDER_H
#include "../ObjectType.h"
#include "../../AABB.h"
#include "../../../Component/Definition/ComponentArgument.h"
struct Triangle;
class GameObject;

struct Collision {

};

class Collider: public ComponentArgument{
public:
    Collider() = default;
    Collider(const Collider& other)  : ComponentArgument(other) {
        this->staticObject = other.staticObject;
    };
    bool staticObject = false;
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
    std::unique_ptr<Collider> collider;
    std::vector<Vector3> vertices;
    std::vector<Triangle> triangles = std::vector<Triangle>();
    AABB boundBox = AABB::Empty();
    void CalculateAABB();

    [[nodiscard]] bool ContainsPoint(const Vector3 &point) const;

    [[nodiscard]] bool IntersectsAABB(const GameObjectArgument &other) const;

    void ExpandAABB(const Vector3 &point);

    void MergeAABB(const AABB &other);
private:
    AABB aabb = AABB::Empty();
    bool shouldUpdateAABB = true;
};



#endif //COLLIDER_H