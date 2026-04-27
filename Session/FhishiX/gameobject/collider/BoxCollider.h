#ifndef BOXCOLLIDER_H
#define BOXCOLLIDER_H

#include "Collider.h"

#include "../../../Component/Definition/Component.h"

class BoxCollider final : public Component<BoxCollider,Collider> {
public:
    Vector3 center = Vector3::Zero();
    Vector3 size = Vector3::Zero();
    mutable bool haveMesh = false;
    BoxCollider(const bool isStatic, const Vector3& center, const Vector3& size)
        : Component(ColliderType::Box, isStatic), center(center), size(size) {
    }

    BoxCollider() : Component(ColliderType::Box, false) {}
    std::unique_ptr<Collider> clone() const override {
        return std::make_unique<BoxCollider>(*this);
    }

    Vector3 CalculateLocalInertia(float mass) const override;
    AABB GetAABB() const override;

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
    void CalculateAABB() const override {

    }


    void ParseFromString(const std::string &arg) override;



#ifdef _WIN64
    Renderer GetRenderer() override;
#endif

};

#endif // BOXCOLLIDER_H
