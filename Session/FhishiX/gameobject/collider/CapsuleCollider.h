//
// Created by white on 25. 11. 14..
//

#ifndef FPSPROJECTSERVER_CAPSULECOLLIDER_H
#define FPSPROJECTSERVER_CAPSULECOLLIDER_H
#include "Collider.h"

class CapsuleCollider : public Collider {
public:
    Vector3 center;
    float height, radius;
    mutable bool haveMesh = false;
    CapsuleCollider(GameObject* owner,const bool isStatic, const Vector3& center = {0,0,0}, const float height = 1, const float radius = 1 ) : Collider(owner,isStatic), center(center), height(height), radius(radius) {}
    ObjectTypeEnum GetType() const override { return ObjectTypeEnum::Capsule; }
    AABB GetAABB() const override {
        AABB aabb = AABB::Empty();

        const auto& tr = gameobject->transform;


        // 1) 로컬 파라미터
        const float halfH_local = height * 0.5f;

        // 2) 스케일 적용
        const float scaledRadius = radius * std::max(tr.scale.x, tr.scale.z);
        const float scaledHalfH  = halfH_local * tr.scale.y;

        // 3) 로컬 top / bottom
        Vector3 localTop    = center + Vector3(0, +scaledHalfH, 0);
        Vector3 localBottom = center + Vector3(0, -scaledHalfH, 0);

        // 4) 회전 및 위치 적용
        Vector3 worldTop    = tr.rotation * localTop    + tr.position;
        Vector3 worldBottom = tr.rotation * localBottom + tr.position;

        // 5) 두 점으로 AABB 만들고 radius 확장
        aabb.min.x = std::min(worldTop.x, worldBottom.x) - scaledRadius;
        aabb.min.y = std::min(worldTop.y, worldBottom.y) - scaledRadius;
        aabb.min.z = std::min(worldTop.z, worldBottom.z) - scaledRadius;

        aabb.max.x = std::max(worldTop.x, worldBottom.x) + scaledRadius;
        aabb.max.y = std::max(worldTop.y, worldBottom.y) + scaledRadius;
        aabb.max.z = std::max(worldTop.z, worldBottom.z) + scaledRadius;

        return aabb;
    }


    std::unique_ptr<Collider> clone() const override {
        return std::make_unique<CapsuleCollider>(*this);
    }

    //CapsuleCollider 복사 생성자(Collider 복사 오버라이딩)
    CapsuleCollider(const CapsuleCollider& other):
        Collider(other.gameobject, other.staticObject),
        center(other.center),
        height(other.height),
        radius(other.radius),
        haveMesh(other.haveMesh)
    {}
};
#endif //FPSPROJECTSERVER_CAPSULECOLLIDER_H