#include "CapsuleCollider.h"
#include "../GameObjectArgument.h"
//
// Created by white on 25. 12. 23..
//
AABB CapsuleCollider::GetAABB() const {
    AABB aabb = AABB::Empty();

    const auto& tr = gameObject->transform;


    // 1) 로컬 파라미터
    const float halfH_local = height * 0.5f;

    // 2) 스케일 적용
    const float scaledRadius = radius * max(tr.scale.x, tr.scale.z);
    const float scaledHalfH  = halfH_local * tr.scale.y;

    // 3) 로컬 top / bottom
    Vector3 localTop    = center + Vector3(0, +scaledHalfH, 0);
    Vector3 localBottom = center + Vector3(0, -scaledHalfH, 0);

    // 4) 회전 및 위치 적용
    Vector3 worldTop    = tr.rotation * localTop    + tr.position;
    Vector3 worldBottom = tr.rotation * localBottom + tr.position;

    // 5) 두 점으로 AABB 만들고 radius 확장
    aabb.min.x = min(worldTop.x, worldBottom.x) - scaledRadius;
    aabb.min.y = min(worldTop.y, worldBottom.y) - scaledRadius;
    aabb.min.z = min(worldTop.z, worldBottom.z) - scaledRadius;

    aabb.max.x = max(worldTop.x, worldBottom.x) + scaledRadius;
    aabb.max.y = max(worldTop.y, worldBottom.y) + scaledRadius;
    aabb.max.z = max(worldTop.z, worldBottom.z) + scaledRadius;

    return aabb;
}