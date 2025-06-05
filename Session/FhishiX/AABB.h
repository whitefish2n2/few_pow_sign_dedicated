//
// Created by white on 25. 5. 20.
//

#ifndef AABB_H
#define AABB_H
#include "Vector3.h"

struct AABB {
    Vector3 min, max;

    static AABB ComputeUnion(const AABB &a, const AABB &b) {
        AABB result = AABB();
        result.min.x = a.min.x<b.min.x ? a.min.x : b.min.x;
        result.min.y = a.min.y<b.min.y ? a.min.y : b.min.y;
        result.min.z = a.min.z<b.min.z ? a.min.z : b.min.z;
        result.max.x = a.max.x>b.max.x ? a.max.x : b.max.x;
        result.max.y = a.max.y>b.max.y ? a.max.y : b.max.y;
        result.max.z = a.max.z>b.max.z ? a.max.z : b.max.z;
        return result;
    };

    static AABB Empty() {
        return AABB{
            Vector3{ std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() },
            Vector3{ std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() }
        };
    }
};
#endif //AABB_H
