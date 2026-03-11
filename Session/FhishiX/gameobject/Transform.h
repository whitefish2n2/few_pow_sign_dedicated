//
// Created by white on 25. 10. 28.
//


#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "../quaternion/Quaternion.h"
#include "../vector/Vector3.h"

class Transform {
    private:
    Matrix4 worldMatrix;
    bool isDirty = true;
    Vector3 position = Vector3::Zero();
    Vector3 eularRotation = Vector3::Zero();
    Quaternion rotation = Quaternion::Identity;
    Vector3 scale = Vector3::One();
    public:
    const Vector3& GetPosition() const { return position; }
    const Vector3& GetEularRotation() const { return eularRotation; }
    const Quaternion& GetRotation() const { return rotation; }
    const Vector3& GetScale() const { return scale; }
    void SetPosition(const Vector3& newPos) {
        position = newPos;
        isDirty = true;
    }

    void SetRotation(const Quaternion& newRot) {
        rotation = newRot;
        eularRotation = newRot.ToEuler();
        isDirty = true;
    }

    void SetScale(const Vector3& newScale) {
        scale = newScale;
        isDirty = true;
    }
    Matrix4 GetWorldMatrix() {
        if (isDirty) {
            // 크기 -> 회전 -> 이동 순으로 행렬 곱셈 (TRS)
            Matrix4 S = Matrix4::Scale(scale);
            Matrix4 R = rotation.ToMatrix4();
            Matrix4 T = Matrix4::Translation(position);

            worldMatrix = T * R * S;;
            isDirty = false;
        }
        return worldMatrix;
    }
};
#endif //TRANSFORM_H
