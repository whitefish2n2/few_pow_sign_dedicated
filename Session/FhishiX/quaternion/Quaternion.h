//
// Created by white on 25. 11. 5.
//

#ifndef QUATERNION_H
#define QUATERNION_H
#pragma once
#include <cmath>
#include "../Vector/Vector3.h"
#include "../Matrix/Matrix4.h"

class Quaternion {
public:
    float w, x, y, z;
    //쿼터니언 기본값
    static const Quaternion Identity;
    Quaternion(float w = 1, float x = 0, float y = 0, float z = 0)
        : w(w), x(x), y(y), z(z) {}

    // 단위화
    Quaternion Normalized() const {
        float mag = std::sqrt(w*w + x*x + y*y + z*z);
        return Quaternion(w / mag, x / mag, y / mag, z / mag);
    }

    // 쿼터니언 곱셈 (회전 누적)
    Quaternion operator*(const Quaternion& q) const {
        return Quaternion(
            w*q.w - x*q.x - y*q.y - z*q.z,
            w*q.x + x*q.w + y*q.z - z*q.y,
            w*q.y - x*q.z + y*q.w + z*q.x,
            w*q.z + x*q.y - y*q.x + z*q.w
        );
    }

    // 벡터 회전
    Vector3 operator*(const Vector3& v) const {
        Quaternion qv(0, v.x, v.y, v.z);
        Quaternion inv = Inverse();
        Quaternion res = (*this) * qv * inv;
        return Vector3(res.x, res.y, res.z);
    }

    // 역쿼터니언 (단위 쿼터니언 기준)
    Quaternion Inverse() const {
        return Quaternion(w, -x, -y, -z);
    }

    //쿼터니언의 크기 제곱
    float MagnitudeSq() const {
        return w * w + x * x + y * y + z * z;
    }

    // 오일러 -> 쿼터니언
    static Quaternion FromEuler(const Vector3& eulerDeg) {
        Vector3 euler = eulerDeg * (3.1415926535f / 180.0f);
        float cx = std::cos(euler.x * 0.5f);
        float sx = std::sin(euler.x * 0.5f);
        float cy = std::cos(euler.y * 0.5f);
        float sy = std::sin(euler.y * 0.5f);
        float cz = std::cos(euler.z * 0.5f);
        float sz = std::sin(euler.z * 0.5f);

        return Quaternion(
            cz * cy * cx + sz * sy * sx,
            cz * cy * sx - sz * sy * cx,
            cz * sy * cx + sz * cy * sx,
            sz * cy * cx - cz * sy * sx
        ).Normalized();
    }

    // 쿼터니언 -> 오일러 (도 단위)
    Vector3 ToEuler() const {
        float sinr_cosp = 2 * (w * x + y * z);
        float cosr_cosp = 1 - 2 * (x * x + y * y);
        float roll = std::atan2(sinr_cosp, cosr_cosp);

        float sinp = 2 * (w * y - z * x);
        float pitch = std::abs(sinp) >= 1 ? std::copysign(3.1415926535f / 2, sinp) : std::asin(sinp);

        float siny_cosp = 2 * (w * z + x * y);
        float cosy_cosp = 1 - 2 * (y * y + z * z);
        float yaw = std::atan2(siny_cosp, cosy_cosp);

        return Vector3(roll, pitch, yaw) * (180.0f / 3.1415926535f);
    }

    //주의: 이 함수를 호출하기 전에 쿼터니언은 반드시 정규화(Normalize())하세요
    Matrix4 ToMatrix4() const {

        float x2 = x + x;
        float y2 = y + y;
        float z2 = z + z;

        float xx = x * x2;
        float xy = x * y2;
        float xz = x * z2;
        float yy = y * y2;
        float yz = y * z2;
        float zz = z * z2;
        float wx = w * x2;
        float wy = w * y2;
        float wz = w * z2;

        Matrix4 result; // Identity로 초기화됨
        // 첫 번째 행 (m[0] ~ m[3])
        result.m[0] = 1.0f - (yy + zz); // m[0][0]
        result.m[1] = xy - wz;          // m[0][1]
        result.m[2] = xz + wy;          // m[0][2]
        result.m[3] = 0.0f;             // m[0][3] (위치 x)

        // 두 번째 행 (m[4] ~ m[7])
        result.m[4] = xy + wz;          // m[1][0]
        result.m[5] = 1.0f - (xx + zz); // m[1][1]
        result.m[6] = yz - wx;          // m[1][2]
        result.m[7] = 0.0f;             // m[1][3] (위치 y)

        // 세 번째 행 (m[8] ~ m[11])
        result.m[8] = xz - wy;          // m[2][0]
        result.m[9] = yz + wx;          // m[2][1]
        result.m[10] = 1.0f - (xx + yy); // m[2][2]
        result.m[11] = 0.0f;            // m[2][3] (위치 z)

        // 네 번째 행 (m[12] ~ m[15]) - (0, 0, 0, 1)로 유지됨
        // result.m[12] = 0.0f;
        // result.m[13] = 0.0f;
        // result.m[14] = 0.0f;
        // result.m[15] = 1.0f;

        return result;
    }


    //오퍼레이터
    Quaternion operator+(const Quaternion& other) const {
        return Quaternion(
            w + other.w,
            x + other.x,
            y + other.y,
            z + other.z
        );
    }
    Quaternion operator-(const Quaternion& other) const {
        return Quaternion(
            w - other.w,
            x - other.x,
            y - other.y,
            z - other.z
        );
    }
    Quaternion operator*(float scalar) const {
        return Quaternion(
            w * scalar,
            x * scalar,
            y * scalar,
            z * scalar
        );
    }
    Quaternion operator=(const Quaternion& other) {
        w = other.w;
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }
};
inline Quaternion operator*(float scalar, const Quaternion& q) {
    return q * scalar;
}

#endif //QUATERNION_H
