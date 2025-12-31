//
// Created by white on 25. 11. 3.
//

#ifndef MESHCOLLIDER_H
#define MESHCOLLIDER_H
#include <memory>
#include <vector>
#include <cpprest/asyncrt_utils.h>
#include "Collider.h"
#include "../../quaternion/Quaternion.h"


class MeshCollider: public Collider {
public:
    mutable std::vector<Vector3> vertices;
    mutable std::vector<int> trianglesIndices;
    MeshCollider(const bool isStatic, const std::vector<Vector3> &vertices, const std::vector<int> &triangles) : Collider(isStatic),vertices(vertices),trianglesIndices(triangles) {
    }
    ObjectTypeEnum GetType() const override { return ObjectTypeEnum::Mesh; }
    const std::vector<Vector3>& GetVertices() const {
        return vertices;
    }
    const std::vector<int>& GetTriangles() const {
        return trianglesIndices;
    }
    AABB GetAABB() const override;
    std::unique_ptr<Collider> clone() const override {
        return std::make_unique<MeshCollider>(*this);
    }
    MeshCollider(const MeshCollider& other):
        Collider(other.staticObject),
        vertices(other.vertices),
        trianglesIndices(other.trianglesIndices)
    {}

    Vector3 GetAABBSize() const override {
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
};
#endif //MESHCOLLIDER_H
