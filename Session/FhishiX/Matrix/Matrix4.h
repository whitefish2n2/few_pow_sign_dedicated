#ifndef MATRIX4_H
#define MATRIX4_H
#include "Matrix.h"
#include "Matrix3.h"

//행 우선 매트릭스4
struct Matrix4 : public Matrix {
    std::array<float, 16> m{1,0,0,0,
                            0,1,0,0,
                            0,0,1,0,
                            0,0,0,1};

    Matrix4() { Matrix4::Identity(); }

    void Identity() override {
        m = {1,0,0,0,
             0,1,0,0,
             0,0,1,0,
             0,0,0,1};
    }

    static Matrix4 Translation(const Vector3& v) {
        Matrix4 r;
        r.m = {1,0,0,v.x,
               0,1,0,v.y,
               0,0,1,v.z,
               0,0,0,1};
        return r;
    }


    static Matrix4 Scale(const Vector3& s) {
        Matrix4 r;
        r.m = {s.x,0,0,0,
               0,s.y,0,0,
               0,0,s.z,0,
               0,0,0,1};
        return r;
    }

    static Matrix4 FromRotation(const Matrix3& rot) {
        Matrix4 r;
        r.m = {
            rot.m[0], rot.m[1], rot.m[2], 0,
            rot.m[3], rot.m[4], rot.m[5], 0,
            rot.m[6], rot.m[7], rot.m[8], 0,
            0,0,0,1
        };
        return r;
    }

    [[nodiscard]] Vector3 TransformPoint(const Vector3& v) const {
        return Vector3(m[0]*v.x + m[1]*v.y + m[2]*v.z + m[3],
            m[4]*v.x + m[5]*v.y + m[6]*v.z + m[7],
            m[8]*v.x + m[9]*v.y + m[10]*v.z + m[11]);
    }

    [[nodiscard]] Matrix4 Transpose() const {
        Matrix4 result = *this;
        // 4x4 행렬에서 대칭 위치에 있는 값만 교환
        std::swap(result.m[1], result.m[4]);  // m[0][1] <-> m[1][0]
        std::swap(result.m[2], result.m[8]);  // m[0][2] <-> m[2][0]
        std::swap(result.m[3], result.m[12]); // m[0][3] <-> m[3][0]

        std::swap(result.m[6], result.m[9]);  // m[1][2] <-> m[2][1]
        std::swap(result.m[7], result.m[13]); // m[1][3] <-> m[3][1]

        std::swap(result.m[11], result.m[14]);// m[2][3] <-> m[3][2]

        return result;
    }


    Matrix4 operator*(const Matrix4& o) const {
        Matrix4 r;
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
                r.m[row*4+col] =
                    m[row*4+0]*o.m[0*4+col] +
                    m[row*4+1]*o.m[1*4+col] +
                    m[row*4+2]*o.m[2*4+col] +
                    m[row*4+3]*o.m[3*4+col];
        return r;
    }
};

#endif // MATRIX4_H
