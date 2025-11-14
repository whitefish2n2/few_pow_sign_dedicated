#include "GameObject.h"
//
// Created by white on 25. 10. 28.
//
void GameObject::CalculateAABB() {
    // 빈 AABB로 초기화
    boundBox = AABB::Empty();

    // vertices가 비어있으면 빈 AABB 유지
    if (vertices.empty()) {
        return;
    }

    // 첫 번째 vertex로 초기화
    boundBox.min = vertices[0];
    boundBox.max = vertices[0];

    // 모든 vertex를 순회하며 min/max 갱신
    for (size_t i = 1; i < vertices.size(); ++i) {
        const Vector3& v = vertices[i];

        // Min 값 갱신
        if (v.x < boundBox.min.x) boundBox.min.x = v.x;
        if (v.y < boundBox.min.y) boundBox.min.y = v.y;
        if (v.z < boundBox.min.z) boundBox.min.z = v.z;

        // Max 값 갱신
        if (v.x > boundBox.max.x) boundBox.max.x = v.x;
        if (v.y > boundBox.max.y) boundBox.max.y = v.y;
        if (v.z > boundBox.max.z) boundBox.max.z = v.z;
    }
}

// AABB 센터 계산
Vector3 GameObject::GetAABBCenter() const {
    return Vector3(
        (boundBox.min.x + boundBox.max.x) * 0.5f,
        (boundBox.min.y + boundBox.max.y) * 0.5f,
        (boundBox.min.z + boundBox.max.z) * 0.5f
    );
}

// AABB 크기 계산
Vector3 GameObject::GetAABBSize() const {
    return Vector3(
        boundBox.max.x - boundBox.min.x,
        boundBox.max.y - boundBox.min.y,
        boundBox.max.z - boundBox.min.z
    );
}

// 점이 AABB 안에 있는지 확인
bool GameObject::ContainsPoint(const Vector3& point) const {
    return (point.x >= boundBox.min.x && point.x <= boundBox.max.x) &&
           (point.y >= boundBox.min.y && point.y <= boundBox.max.y) &&
           (point.z >= boundBox.min.z && point.z <= boundBox.max.z);
}

// 두 AABB가 겹치는지 확인
bool GameObject::IntersectsAABB(const GameObject& other) const {
    return (boundBox.min.x <= other.boundBox.max.x && boundBox.max.x >= other.boundBox.min.x) &&
           (boundBox.min.y <= other.boundBox.max.y && boundBox.max.y >= other.boundBox.min.y) &&
           (boundBox.min.z <= other.boundBox.max.z && boundBox.max.z >= other.boundBox.min.z);
}

// AABB 확장 (새로운 점 추가)
void GameObject::ExpandAABB(const Vector3& point) {
    if (point.x < boundBox.min.x) boundBox.min.x = point.x;
    if (point.y < boundBox.min.y) boundBox.min.y = point.y;
    if (point.z < boundBox.min.z) boundBox.min.z = point.z;

    if (point.x > boundBox.max.x) boundBox.max.x = point.x;
    if (point.y > boundBox.max.y) boundBox.max.y = point.y;
    if (point.z > boundBox.max.z) boundBox.max.z = point.z;
}

// AABB를 다른 AABB와 병합
void GameObject::MergeAABB(const AABB& other) {
    boundBox = AABB::ComputeUnion(boundBox, other);
}