//
// Created by white on 25. 5. 20.
//

#ifndef OBJECT_H
#define OBJECT_H
#include <string>
#include <vector>
#include <memory>
#include "ObjectType.h"
#include "Transform.h"
#include "../Vector/Vector3.h"
#include "../Triangle.h"
#include "../ObjectTag.h"
#include "../AABB.h"
#include "../Layer.h"
#include "Collider/Collider.h"

class GameObject {
    public:
    bool enablePhysics = true;
    GameObject(const GameObject&){}
    GameObject(){}


    std::string id = "";
    TagEnum tag = TagEnum::Untagged;
    ObjectTypeEnum type = ObjectTypeEnum::Undefined;
    Layers layer = Layers::Default;
    std::unique_ptr<Collider> collider;
    std::vector<Vector3> vertices;
    std::vector<Triangle> triangles = std::vector<Triangle>();
    AABB boundBox = AABB::Empty();
    Transform transform;

    void CalculateAABB();

    Vector3 GetAABBCenter() const;

    Vector3 GetAABBSize() const;

    bool ContainsPoint(const Vector3 &point) const;

    bool IntersectsAABB(const GameObject &other) const;

    void ExpandAABB(const Vector3 &point);

    void MergeAABB(const AABB &other);

    GameObject operator=(const GameObject & target) {
        id = target.id;
        tag = target.tag;
        transform = target.transform;
        collider = target.collider->clone();
        vertices = target.vertices;
        layer = target.layer;
        return *this;
    }
};
#endif //OBJECT_H
