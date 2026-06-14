//
// Created by white on 25. 5. 20.
//

#ifndef AABB_H
#define AABB_H
#include "vector/Vector3.h"
#include <limits>

struct AABB {
    Vector3 min, max;

    ///AABB a와 AABB b를 합쳐서 새로운 aabb 반환
    static AABB ComputeUnion(const AABB &a, const AABB &b) {
        AABB result = AABB();
        result.min.x = a.min.x<b.min.x ? a.min.x : b.min.x;
        result.min.y = a.min.y<b.min.y ? a.min.y : b.min.y;
        result.min.z = a.min.z<b.min.z ? a.min.z : b.min.z;
        result.max.x = a.max.x>b.max.x ? a.max.x : b.max.x;
        result.max.y = a.max.y>b.max.y ? a.max.y : b.max.y;
        result.max.z = a.max.z>b.max.z ? a.max.z : b.max.z;//끔찍..
        return result;
    };

    AABB Merge(const AABB &other) {
        AABB result = AABB();
        result.min.x = this->min.x<other.min.x ? this->min.x : other.min.x;
        result.min.y = this->min.y<other.min.y ? this->min.y : other.min.y;
        result.min.z = this->min.z<other.min.z ? this->min.z : other.min.z;
        result.max.x = this->max.x>other.max.x ? this->max.x : other.max.x;
        result.max.y = this->max.y>other.max.y ? this->max.y : other.max.y;
        result.max.z = this->max.z>other.max.z ? this->max.z : other.max.z;
        return result;
    }

    static bool Intersects(const AABB &a, const AABB &b) {
        return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
               (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
               (a.min.z <= b.max.z && a.max.z >= b.min.z);
    }

    bool Intersects(const AABB &other) const {
        return (this->min.x <= other.max.x && this->max.x >= other.min.x) &&
               (this->min.y <= other.max.y && this->max.y >= other.min.y) &&
               (this->min.z <= other.max.z && this->max.z >= other.min.z);
    }

    bool IntersectsSphere(const Vector3& center, float radius) const {
        // 구의 중심을 AABB 영역 내로 클램핑하여 가장 가까운 점을 찾음
        float closestX = std::clamp(center.x, min.x, max.x);
        float closestY = std::clamp(center.y, min.y, max.y);
        float closestZ = std::clamp(center.z, min.z, max.z);

        // 가장 가까운 점과 구의 중심 사이의 거리 제곱 계산
        float distanceX = center.x - closestX;
        float distanceY = center.y - closestY;
        float distanceZ = center.z - closestZ;

        float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY) + (distanceZ * distanceZ);

        // 그 거리가 반지름의 제곱보다 작거나 같으면 충돌!
        return distanceSquared <= (radius * radius);
    }

    ///텅텅 AABB
    static AABB Empty() {
        const float float_max = (std::numeric_limits<float>::max)();
        const float float_lowest = (std::numeric_limits<float>::lowest)();

        return AABB{
            Vector3{ float_max, float_max, float_max },
            Vector3{ float_lowest, float_lowest, float_lowest }
        };
    }
};
#endif //AABB_H
