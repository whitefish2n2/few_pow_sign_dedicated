//
// Created by white on 25. 5. 23.
//

#ifndef COLLIDER_H
#define COLLIDER_H
#define WIN32_LEAN_AND_MEAN
#include <memory>
#include <vector>

#include "ColliderMaterial.h"
#include "CollisionSolver.h"
#include "../../AABB.h"
#include "../../Layer.h"

#include "../../../Component/Definition/ComponentArgument.h"
#include "../../../FhishiX/Triangle.h"

#ifdef _WIN64
#include "../../Renderer.h"
#endif
struct Triangle;
class GameObject;

struct Collision {

};

class Collider : public ComponentArgument{
protected:
    mutable uint32_t transformRotVersion = -1;
    mutable uint32_t transformScaleVersion = -1;
    mutable uint32_t transformPosVersion = -1;
    mutable Vector3 lastPosition = Vector3(0, 0, 0);

    //rot과 scale이 적용된 AABB 캐싱
    mutable AABB cachedAABB;

    ColliderType shapeType = ColliderType::None;
public:
    Collider() = default;
    explicit Collider(ColliderType type,bool isStatic):shapeType(type),staticObject(isStatic){};
    virtual ~Collider() noexcept = default;
    bool staticObject = false;
    bool isTrigger = false;
    Layer cachedLayer;
    ColliderMaterial material;///todo: 나중에 공용 ColliderMaterial 저장소 만들어서 여기서 id로 접근할 수 있게 하기
    inline ColliderType GetShapeType() const { return shapeType; }
    ///관성 텐서 계산 함수 - > 구현필
    virtual Vector3 CalculateLocalInertia(float mass) const = 0;

    void Start() override;

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
};



#endif //COLLIDER_H