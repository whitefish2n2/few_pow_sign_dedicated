//
// Created by white on 25. 5. 23.
//

#ifndef COLLIDER_H
#define COLLIDER_H
#include <memory>
#include <vector>

#include "../ObjectType.h"
#include "../../AABB.h"
#include "../../../Component/Definition/ComponentArgument.h"
#include "../../../FhishiX/Triangle.h"

struct Triangle;
class GameObject;

struct Collision {

};

class Collider: public ComponentArgument{
public:
    Collider() = default;
    Collider(bool isStatic):staticObject(isStatic){};
    Collider(const Collider& other);
    virtual ~Collider() noexcept = default;
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
    std::vector<Vector3> vertices;
    std::vector<Triangle> triangles = std::vector<Triangle>();
    AABB boundBox = AABB::Empty();
    void CalculateAABB();

    [[nodiscard]]virtual bool ContainsPoint(const Vector3 &point) const=0;

    [[nodiscard]]virtual bool IntersectsAABB(const GameObjectArgument &other) const=0;

    virtual void ExpandAABB(const Vector3 &point) const=0;

    virtual void MergeAABB(const AABB &other) const=0;
private:
    AABB aabb = AABB::Empty();
    bool shouldUpdateAABB = true;
};



#endif //COLLIDER_H