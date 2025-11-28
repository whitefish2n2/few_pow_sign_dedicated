//
// Created by white on 25. 5. 11.
//

#ifndef VECTOR2_H
#define VECTOR2_H
#include <cmath>

struct Vector2 {
    float x, y;

    ///벡터 길이
    float length() const {
        float fx = static_cast<float>(x);
        float fy = static_cast<float>(y);
        return std::sqrt(fx*fx + fy*fy);
    }

    ///비교용 벡터 길이
    float lengthSquared() const {
        float fx = static_cast<float>(x);
        float fy = static_cast<float>(y);
        return fx*fx + fy*fy;
    }

    Vector2 normalize() const {
        const float len = length();
        if (len == 0.0f) return Vector2::Zero();
        return { x / len, y / len };
    }
    ///vector2의 x,y가 1 이하 -1 이상이 되게 해요
    Vector2 directionlize() const
    {
        const float nx = (x > 0) - (x < 0);
        const float ny = (y > 0) - (y < 0);
        return { nx, ny };
    }
    ///내적해요
    float dot(const Vector2& o) const {
        return static_cast<float>(x)*o.x + static_cast<float>(y)*o.y;
    }

    /// 외적(방향성이 왼쪽이면 양수, 오른쪽이면 음수)
    float cross(const Vector2& v) const {
        return x * v.y - y * v.x;
    }

    static Vector2 Lerp(const Vector2& from, const Vector2& to, float t) {
        return from + (to - from) * t;
    }

    Vector2 perpendicular() const {
        return { -y, x };
    }

    //방향 벡터들
#pragma region directionVector
    static constexpr Vector2 Zero() {
        return { 0, 0};
    }
    static constexpr Vector2 Up() {
        return { 0, 1};
    }
    static constexpr Vector2 Down() {
        return { 0, -1};
    }
    static constexpr Vector2 Left() {
        return  { -1, 0};
    }
    static constexpr Vector2 Right() {
        return { 1, 0};
    }
#pragma endregion

    //오퍼레이터들
#pragma region operators
    Vector2 operator+(const Vector2& o) const {
        return { x + o.x, y + o.y };
    }

    Vector2 operator-(const Vector2& o) const {
        return { x - o.x, y - o.y };
    }

    Vector2 operator*(float s) const {
        return { x * s, y * s };
    }

    Vector2 operator/(float s) const {
        return { x / s, y / s };
    }

    Vector2& operator+=(const Vector2& o) {
        x += o.x; y += o.y;
        return *this;
    }

    Vector2& operator-=(const Vector2& o) {
        x -= o.x; y -= o.y;
        return *this;
    }

    bool operator==(const Vector2& o) const {
        return x == o.x && y == o.y;
    }

    bool operator!=(const Vector2& o) const {
        return !(*this == o);
    }
#pragma endregion
};
#endif //VECTOR2_H
