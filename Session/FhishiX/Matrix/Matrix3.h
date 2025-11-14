#ifndef MATRIX3_H
#define MATRIX3_H

#include <array>

#include "Matrix.h"

struct Matrix3 : public Matrix {
    std::array<float, 9> m{1,0,0,
                           0,1,0,
                           0,0,1};

    Matrix3() { Matrix3::Identity(); }

    void Identity() override {
        m = {1,0,0,
             0,1,0,
             0,0,1};
    }

    static Matrix3 RotationX(float rad) {
        Matrix3 r;
        float c = std::cos(rad), s = std::sin(rad);
        r.m = {1,0,0,
               0,c,-s,
               0,s,c};
        return r;
    }

    static Matrix3 RotationY(float rad) {
        Matrix3 r;
        float c = std::cos(rad), s = std::sin(rad);
        r.m = {c,0,s,
               0,1,0,
               -s,0,c};
        return r;
    }

    static Matrix3 RotationZ(float rad) {
        Matrix3 r;
        float c = std::cos(rad), s = std::sin(rad);
        r.m = {c,-s,0,
               s,c,0,
               0,0,1};
        return r;
    }

    Vector3 operator*(const Vector3& v) const {
        return {
            m[0]*v.x + m[1]*v.y + m[2]*v.z,
            m[3]*v.x + m[4]*v.y + m[5]*v.z,
            m[6]*v.x + m[7]*v.y + m[8]*v.z
        };
    }

    Matrix3 operator*(const Matrix3& o) const {
        Matrix3 r;
        for (int row = 0; row < 3; ++row)
            for (int col = 0; col < 3; ++col)
                r.m[row*3+col] =
                    m[row*3+0]*o.m[0*3+col] +
                    m[row*3+1]*o.m[1*3+col] +
                    m[row*3+2]*o.m[2*3+col];
        return r;
    }
};

#endif
