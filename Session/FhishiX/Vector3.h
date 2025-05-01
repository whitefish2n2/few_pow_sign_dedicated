//
// Created by user on 25. 4. 24.
//

#ifndef VECTOR3_H
#define VECTOR3_H
#include "cmath"
struct Vector3 {
    float x, y, z;
    Vector3 normalize() const {
        const float len = sqrt(x * x + y * y + z * z);
        if (len == 0) return *this;
        return { x / len, y / len, z / len };
    }
};
#endif //VECTOR3_H
