//
// Created by user on 25. 4. 24.
//

#ifndef VECTOR3_H
#define VECTOR3_H
#include "cmath"

#include "Vector2.h"
struct Vector3 {
    float x, y, z;
    Vector3 normalize() const {
        const float len = sqrt(x * x + y * y + z * z);
        if (len == 0) return *this;
        return { x / len, y / len, z / len };
    }
    float Distance(const Vector3 &other) const
    {
        return sqrt(pow (x-other.x,2)+pow(y-other.y,2)+pow (z-other.z,2));
    }
    //오퍼레이터들
    #pragma region Vector3 operators
    Vector3 operator+(const Vector3& p) const
    {
        return { x + p.x, y + p.y, z + p.z };
    }
    Vector3 operator+=(const Vector3& p)
    {
        x += p.x;
        y += p.y;
        z += p.z;
        return *this;
    }
    Vector3 operator-(const Vector3& p) const
    {
        return { x - p.x, y - p.y, z - p.z };
    }
    Vector3 operator-=(const Vector3& p)
    {
        x -= p.x;
        y -= p.y;
        z -= p.z;
        return *this;
    }
    Vector3 operator*(float s) const
    {
        return { x * s, y * s, z * s };
    }
    Vector3 operator*=(float s)
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    Vector3 operator/(float s) const
    {
        return { x / s, y / s, z / s };
    }
    Vector3 operator/=(float s)
    {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }
    Vector3 operator*=(const Vector3& p)
    {
        x *= p.x;
        y *= p.y;
        z *= p.z;
        return *this;
    }

    Vector3 operator+(const Vector2<int>& p)const
    {
        return {x+p.x,y+p.y,z};
    }
    Vector3 operator+=(const Vector2<int>& p)
    {
        x += p.x;
        z += p.y;
        return *this;
    }
    Vector3 operator-(const Vector2<int>& p)const
    {
        return {x-p.x,y-p.y,z};
    }
    Vector3 operator-=(const Vector2<int>& p)
    {
        x -= p.x;
        z -= p.y;
        return *this;
    }

#pragma endregion
    //방향 벡터들
    #pragma region directionVector
    static Vector3 Zero() {
        static Vector3 v = { 0, 0, 0 };
        return v;
    }
    static Vector3 Up() {
        static Vector3 v = { 0, 1, 0 };
        return v;
    }
    static Vector3 Down() {
        static Vector3 v = { 0, -1, 0 };
        return v;
    }
    static Vector3 Forward() {
        static Vector3 v = { 0, 0, 1 };
        return v;
    }
    static Vector3 Back() {
        static Vector3 v = { 0, 0, -1 };
        return v;
    }
    static Vector3 Left() {
        static Vector3 v = { -1, 0, 0 };
        return v;
    }
    static Vector3 Right() {
        static Vector3 v = { 1, 0, 0 };
        return v;
    }
    #pragma endregion

};
static Vector3 zeroVector = Vector3(0, 0, 0);
#endif //VECTOR3_H
