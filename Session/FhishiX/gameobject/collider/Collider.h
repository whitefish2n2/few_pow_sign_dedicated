//
// Created by white on 25. 5. 23.
//

#ifndef COLLIDER_H
#define COLLIDER_H
#define WIN32_LEAN_AND_MEAN
#include <memory>
#include <vector>

#include "../../AABB.h"

#include "../../../Component/Definition/ComponentArgument.h"
#include "../../../FhishiX/Triangle.h"

#ifdef _WIN64
#include "../../Renderer.h"
#endif
struct Triangle;
class GameObject;

struct Collision {

};

class Collider: public ComponentArgument{
public:
    Collider() = default;
    explicit Collider(bool isStatic):staticObject(isStatic){};
    virtual ~Collider() noexcept = default;
    bool staticObject = false;
    bool isTrigger = false;
    [[nodiscard]] virtual std::unique_ptr<Collider> clone() const = 0;

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
    virtual void CalculateAABB() const  = 0;

    [[nodiscard]]virtual bool ContainsPoint(const Vector3 &point) const=0;

    [[nodiscard]]virtual bool IntersectsAABB(const GameObjectArgument &other) const=0;

    virtual void ExpandAABB(const Vector3 &point) const=0;

    virtual void MergeAABB(const AABB &other) const=0;

    #ifdef _WIN64
    int rendererIndex = -1;
    virtual Renderer GetRenderer() = 0;
    void OnAttach() override;
    #endif
private:
    AABB aabb = AABB::Empty();
    bool shouldUpdateAABB = true;
};



#endif //COLLIDER_H