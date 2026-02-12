//
// Created by user on 25. 4. 24.
//

#ifndef VECTOR3_H
#define VECTOR3_H
#include <cmath>
#include <string>
#include "Vector2.h"
//todo: eigen-like 최적화 적용하기
///warning: 가상함수가 들어가선 안되는 구조체
struct Vector3 {
    float x, y, z;

    ///Format: "F4,F4,F4"
    ///문자열을 Vector3으로 변환, 파싱 실패시 Vector3::Zero()반환
    static Vector3 ParseVector3(const std::string& str) {
        float x, y, z;
        // C# 포맷이 "F4,F4,F4" (콤마 구분)이므로 sscanf로 파싱
        if(sscanf_s(str.c_str(), "%f,%f,%f", &x, &y, &z) == 3) {
            return Vector3(x, y, z);
        }
        return Vector3::Zero();
    }
    ///정규화(방향 벡터화) 된 벡터 반환
    Vector3 Normalized() const {
        const float len = this->length();
        if (len == 0) return *this;
        return { x / len, y / len, z / len };
    }

    ///정규화(방향 벡터화)
    void Normalize() {
        const float len = this->length();
        if (len!=0) {
            x /= len;y /= len;z /= len;
        }
    }

    ///other와 떨어져있는 거리 계산
    float Distance(const Vector3 &other) const
    {
        const float dx = x - other.x;
        const float dy = y - other.y;
        const float dz = z - other.z;
        return sqrt(dx*dx + dy*dy + dz*dz);
    }

    ///벡터 길이
    float length() const {
        return sqrt(x*x + y*y + z*z);
    }

    ///비교용 벡터 길이
    float lengthSquared() const {
        return x*x + y*y + z*z;
    }

    ///벡터 내적
    float dot(const Vector3& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    ///벡터 외적
    [[nodiscard]] Vector3 cross(const Vector3& v) const {
        return {
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        };
    }

    ///반사 벡터
    Vector3 Reflect(const Vector3& normal) const {
        return *this - normal * (2.0f * this->dot(normal));
    }

    ///투영 벡터
    Vector3 Project(const Vector3& on) const {
        static constexpr float EPS = 1e-8f;
        float d = dot(on);
        float lenSq = on.lengthSquared();
        if (lenSq<EPS) return Vector3::Zero();
        return on * (d / lenSq);
    }

    //선형 보간
    static Vector3 Lerp(const Vector3& from, const Vector3& to, float t) {
        return from + (to - from) * t;
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
    Vector3 operator+(const Vector2& p)const
    {
        return {x+p.x,y+p.y,z};
    }
    Vector3 operator+=(const Vector2& p)
    {
        x += p.x;
        y += p.y;
        return *this;
    }
    Vector3 operator-(const Vector2& p)const
    {
        return {x-p.x,y-p.y,z};
    }
    Vector3 operator-=(const Vector2& p)
    {
        x -= p.x;
        y -= p.y;
        return *this;
    }

    bool operator==(const Vector3& v) const {
        static constexpr float EPS = 1e-5f;
        return std::fabs(x - v.x) < EPS &&
               std::fabs(y - v.y) < EPS &&
               std::fabs(z - v.z) < EPS;
    }

    bool operator!=(const Vector3& v) const {
        return !(*this == v);
    }
#pragma endregion
    //방향 벡터들
    #pragma region directionVector
    static constexpr Vector3 Zero() {
        return { 0, 0, 0 };
    }
    static constexpr Vector3 Up() {
        return { 0, 1, 0 };
    }
    static constexpr Vector3 Down() {
        return { 0, -1, 0 };
    }
    static constexpr Vector3 Forward() {
        return  { 0, 0, 1 };
    }
    static constexpr Vector3 Back() {
        return  { 0, 0, -1 };
    }
    static constexpr Vector3 Left() {
        return  { -1, 0, 0 };
    }
    static constexpr Vector3 Right() {
        return { 1, 0, 0 };
    }
    #pragma endregion

};

inline Vector3 operator*(float s, const Vector3& v) {
    return v * s;
}
#endif //VECTOR3_H
