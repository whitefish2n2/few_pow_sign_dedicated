//
// Created by white on 25. 11. 3.
//

#ifndef MESHCOLLIDER_H
#define MESHCOLLIDER_H
#include "Collider.h"


class MeshCollider: public Collider {
public:
    mutable std::vector<Vector3> vertices;
    mutable std::vector<int> triangles;
    MeshCollider(GameObject* owner,const bool isStatic, const std::vector<Vector3> &vertices, const std::vector<int> &triangles) : Collider(owner,isStatic),vertices(vertices),triangles(triangles) {
    }

    ObjectTypeEnum GetType() const override { return ObjectTypeEnum::Box; }
    const std::vector<Vector3>& GetVertices() {
        return vertices;
    }
    const std::vector<int>& GetTriangles() {
        return triangles;
    }
    AABB GetAABB() const override {
        const auto& verts = GetVertices();
        AABB aabb = AABB::Empty();
        aabb.min = verts[0];
        aabb.max = verts[0];

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
    virtual const std::vector<Vector3>& GetVertices() const = 0;
    virtual const std::vector<Triangle>& GetTriangles() const = 0;
};
#endif //MESHCOLLIDER_H
