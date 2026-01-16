//
// Created by white on 25. 12. 23..
//
#include "MeshCollider.h"

#include "../GameObject.h"
#include "../../quaternion/Quaternion.h"
#include "../GameObjectArgument.h"
#include "../../../Component/Definition/ComponentFactory.h"

AABB MeshCollider::GetAABB() const {
    const auto& verts = GetVertices();
    AABB aabb = AABB::Empty();
    aabb.min = verts[0];
    aabb.max = verts[0];
    const Vector3 pos   =  gameObject-> transform.position;
    const Vector3 scale = gameObject->transform.scale;
    const Quaternion rot = gameObject->transform.rotation;


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
REGISTER_COMPONENT(MeshCollider)