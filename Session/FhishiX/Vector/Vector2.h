//
// Created by white on 25. 5. 11.
//

#ifndef VECTOR2_H
#define VECTOR2_H
#include <cmath>

template<typename T>
struct Vector2 {
    T x, y;
    Vector2<float> normalize() const {
        auto fx = static_cast<float>(x);
        auto fy = static_cast<float>(y);
        float len = std::sqrt(fx * fx + fy * fy);
        if (len == 0.0f) return { fx, fy };
        return { fx / len, fy / len };
    }
    ///<summary>vector2<int>의 x,y가 1 이하 -1 이상이 되게 해요</summary>
    Vector2<int> directionlize() const
    {
        int nx = (x > 0) - (x < 0);
        int ny = (y > 0) - (y < 0);
        return { nx, ny };
    }
};
using FloatVector2 = Vector2<float>;
using IntVector2 = Vector2<int>;
#endif //VECTOR2_H
