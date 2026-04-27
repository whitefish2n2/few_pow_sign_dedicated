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
    bool isMatrixDirty = true;
    uint32_t positionVersion = 1;
    uint32_t rotationVersion = 1;
    uint32_t scaleVersion = 1;
    Vector3 position = Vector3::Zero();
    Vector3 eularRotation = Vector3::Zero();
    Quaternion rotation = Quaternion::Identity;
    Vector3 scale = Vector3::One();
    public:
    ///이전에 GetPosVersion 했을떄와 지금 version이 다르면 상태가 이전과 다르다는 것
    uint32_t GetPosVersion() const { return positionVersion; }
    uint32_t GetRotVersion() const { return rotationVersion; }
    uint32_t GetScaleVersion() const { return scaleVersion; }
    const Vector3& GetPosition() const { return position; }
    const Vector3& GetEularRotation() const { return eularRotation; }
    const Quaternion& GetRotation() const { return rotation; }
    const Vector3& GetScale() const { return scale; }
    void SetPosition(const Vector3& newPos) {
        position = newPos;
        isMatrixDirty = true;
        positionVersion++;
    }

    void SetRotation(const Quaternion& newRot) {
        rotation = newRot;
        eularRotation = newRot.ToEuler();
        isMatrixDirty = true;
        rotationVersion++;
    }
    void SetScale(const Vector3& newScale) {
        scale = newScale;
        isMatrixDirty = true;
        scaleVersion++;
    }
    Matrix4 GetWorldMatrix() {
        if (isMatrixDirty) {
            // 크기 -> 회전 -> 이동 순으로 행렬 곱셈 (TRS)
            Matrix4 S = Matrix4::Scale(scale);
            Matrix4 R = rotation.ToMatrix4();
            Matrix4 T = Matrix4::Translation(position);

            worldMatrix = T * R * S;;
            isMatrixDirty = false;
        }
        return worldMatrix;
    }
};
#endif //TRANSFORM_H
