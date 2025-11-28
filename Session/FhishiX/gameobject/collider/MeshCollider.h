//
// Created by white on 25. 11. 3.
//

#ifndef MESHCOLLIDER_H
#define MESHCOLLIDER_H
#include <memory>
#include <vector>
#include <cpprest/asyncrt_utils.h>

#include "Collider.h"
#include "../GameObject.h"


class MeshCollider: public Collider {
public:
    mutable std::vector<Vector3> vertices;
    mutable std::vector<int> triangles;
    MeshCollider(GameObject* owner,const bool isStatic, const std::vector<Vector3> &vertices, const std::vector<int> &triangles) : Collider(owner,isStatic),vertices(vertices),triangles(triangles) {
    }
    ObjectTypeEnum GetType() const override { return ObjectTypeEnum::Mesh; }
    const std::vector<Vector3>& GetVertices() const {
        return vertices;
    }
    const std::vector<int>& GetTriangles() const {
        return triangles;
    }
    AABB GetAABB() const override {
        const auto& verts = GetVertices();
        AABB aabb = AABB::Empty();
        aabb.min = verts[0];
        aabb.max = verts[0];
        const Vector3 pos   = gameobject->transform.position;
        const Vector3 scale = gameobject->transform.scale;
        const Quaternion rot = gameobject->transform.rotation;


        for (size_t i = 1; i < verts.size(); ++i) {
            const Vector3& v = verts[i];

            if (v.x < aabb.min.x) aabb.min.x = v.x;
            if (v.y < aabb.min.y) aabb.min.y = v.y;
            if (v.z < aabb.min.z) aabb.min.z = v.z;

            if (v.x > aabb.max.x) aabb.max.x = v.x;
            if (v.y > aabb.max.y) aabb.max.y = v.y;
            if (v.z > aabb.max.z) aabb.max.z = v.z;
        }

        return aabb;
    }
    std::unique_ptr<Collider> clone() const override {
        return std::make_unique<MeshCollider>(*this);
    }
    MeshCollider(const MeshCollider& other):
        Collider(other.gameobject, other.staticObject),
        vertices(other.vertices),
        triangles(other.triangles)
    {}
};
#endif //MESHCOLLIDER_H
