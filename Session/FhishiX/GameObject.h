//
// Created by white on 25. 5. 20.
//

#ifndef OBJECT_H
#define OBJECT_H
#include <string>
#include <vector>

#include "Vector3.h"
#include "triangle.h"
#include "ObjectTag.h"
#include "AABB.h"
#include "Layer.h"

class GameObject {
    public:
    std::string id;
    ObjectTag::TagEnum tag;
    Layer layer;
    std::vector<Vector3> vertices;
    std::vector<Triangle> triangles = std::vector<Triangle>();
    AABB boundBox;
};
#endif //OBJECT_H
