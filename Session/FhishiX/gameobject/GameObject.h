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
#include "../vector/Vector3.h"
#include "../Triangle.h"
#include "../ObjectTag.h"
#include "../AABB.h"
#include "../Layer.h"
#include "collider/Collider.h"

class GameObject {
    public:
    bool enablePhysics = true;
    GameObject(const GameObject&){}
    GameObject()= default;


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

    [[nodiscard]] Vector3 GetAABBCenter() const;

    [[nodiscard]] Vector3 GetAABBSize() const;

    [[nodiscard]] bool ContainsPoint(const Vector3 &point) const;

    [[nodiscard]] bool IntersectsAABB(const GameObject &other) const;

    void ExpandAABB(const Vector3 &point);

    void MergeAABB(const AABB &other);

    GameObject& operator=(const GameObject & target) {
        if (this == &target)
            return *this;
        this->id = target.id;
        this->tag = target.tag;
        this->transform = target.transform;
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
};
#endif //OBJECT_H
