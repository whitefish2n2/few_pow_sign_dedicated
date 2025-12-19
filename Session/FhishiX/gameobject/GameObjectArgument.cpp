#include "GameObjectArgument.h"

#include "../AABB.h"
#include "../../Component/Definition/ComponentManager.h"
//
// Created by white on 25. 10. 28.
//

GameObject GameObjectArgument::MakeHandle() const {
    return {id,generationId};
}


void GameObjectArgument::CalculateAABB() {
    boundBox = this->collider->GetAABB();
}

// AABB 센터 계산
Vector3 GameObjectArgument::GetAABBCenter() const {
    return this->collider->GetAABBCenter();
}

// AABB 크기 계산
Vector3 GameObjectArgument::GetAABBSize() const {
    return this->collider->GetAABBSize();
}

// 점이 AABB 안에 있는지 확인
bool GameObjectArgument::ContainsPoint(const Vector3& point) const {
    return this->collider->AABBContainsPoint(point);
}

// 두 AABB가 겹치는지 확인
bool GameObjectArgument::IntersectsAABB(const GameObjectArgument& other) const {
    return (boundBox.min.x <= other.boundBox.max.x && boundBox.max.x >= other.boundBox.min.x) &&
           (boundBox.min.y <= other.boundBox.max.y && boundBox.max.y >= other.boundBox.min.y) &&
           (boundBox.min.z <= other.boundBox.max.z && boundBox.max.z >= other.boundBox.min.z);
}

// AABB 확장 (새로운 점 추가)
void GameObjectArgument::ExpandAABB(const Vector3& point) {
    if (point.x < boundBox.min.x) boundBox.min.x = point.x;
    if (point.y < boundBox.min.y) boundBox.min.y = point.y;
    if (point.z < boundBox.min.z) boundBox.min.z = point.z;

    if (point.x > boundBox.max.x) boundBox.max.x = point.x;
    if (point.y > boundBox.max.y) boundBox.max.y = point.y;
    if (point.z > boundBox.max.z) boundBox.max.z = point.z;
}

// AABB를 다른 AABB와 병합
void GameObjectArgument::MergeAABB(const AABB& other) {
    boundBox = AABB::ComputeUnion(boundBox, other);
}

GameObjectArgument & GameObjectArgument::operator=(const GameObjectArgument &target)  {
    if (this == &target)
        return *this;
    this->id = target.id;
    this->generationId = target.generationId;
    this->tag = target.tag;
    this->transform = target.transform;
    for (auto component : target.components ) {
        this->components.push_back(component);
    }
    if (target.collider) {
        this->collider = target.collider->clone();
        this->collider->gameobject = this;
    }
    else
        collider.reset();
    this->vertices = target.vertices;
    this->layer = target.layer;
    return *this;
}
