//
// Created by white on 26. 2. 6..
//
#pragma once
#ifdef _WIN64
#pragma once
#include <DirectXMath.h>

struct Camera {
    using XMVECTOR = DirectX::XMVECTOR;
    using XMFLOAT3 = DirectX::XMFLOAT3;
    using XMMATRIX = DirectX::XMMATRIX;

    XMFLOAT3 pos{};
    XMFLOAT3 look{};
    XMFLOAT3 up{};


    XMMATRIX view{};
    float speed = 0.05f; // 현재 속도
    float defaultSpeed = 0.05f;
    float sprintSpeed = 0.2f;
    void ChangeToDefaultSpeed() {
        speed = defaultSpeed;
    }
    void ChangeToSprintSpeed() {
        speed = sprintSpeed;
    }

    float mouseSpeed = 0.002f;
    float yaw;
    float pitch;

    Camera() {
        pos = { 0.0f, 2.0f, -5.0f }; // 초기 위치
        look = { 0.0f, 0.0f, 1.0f }; // 초기 바라보는 방향 (Z+)
        up = { 0.0f, 1.0f, 0.0f };   // 머리 위 방향 (Y+)
        UpdateView();
    }
    void OnMouseInput(float dx, float dy) {
        yaw += dx * mouseSpeed;
        pitch += dy * mouseSpeed;

        // [고개 꺾임 방지] 위아래 90도 이상 못 꺾게 제한 (짐벌 락 방지)
        if (pitch > 1.5f) pitch = 1.5f;
        if (pitch < -1.5f) pitch = -1.5f;
    }

    void UpdateView() {
        float x = sinf(yaw) * cosf(pitch);
        float y = sinf(pitch);
        float z = cosf(yaw) * cosf(pitch);
        look = { x, y, z };
        XMVECTOR vLook = DirectX::XMLoadFloat3(&look);
        vLook = DirectX::XMVector3Normalize(vLook);
        DirectX::XMStoreFloat3(&look, vLook);

        // 2. 뷰 행렬 생성 (기존과 동일)
        XMVECTOR vPos = DirectX::XMLoadFloat3(&pos);
        XMVECTOR vUp = DirectX::XMLoadFloat3(&up);
        XMVECTOR vTarget = DirectX::XMVectorAdd(vPos, vLook);

        view = DirectX::XMMatrixLookAtLH(vPos, vTarget, vUp);
    }

    void MoveForward() {
        XMVECTOR vPos = DirectX::XMLoadFloat3(&pos);
        XMVECTOR vLook = DirectX::XMLoadFloat3(&look);

        // Pos = Pos + (Look * Speed)
        XMVECTOR vVelocity = DirectX::XMVectorScale(vLook, speed);
        vPos = DirectX::XMVectorAdd(vPos, vVelocity);

        DirectX::XMStoreFloat3(&pos, vPos);
    }

    void MoveBackward() {
        XMVECTOR vPos = DirectX::XMLoadFloat3(&pos);
        XMVECTOR vLook = DirectX::XMLoadFloat3(&look);

        // Pos = Pos - (Look * Speed)
        XMVECTOR vVelocity = DirectX::XMVectorScale(vLook, speed);
        vPos = DirectX::XMVectorSubtract(vPos, vVelocity);

        DirectX::XMStoreFloat3(&pos, vPos);
    }

    void MoveRight() {
        XMVECTOR vPos = DirectX::XMLoadFloat3(&pos);
        XMVECTOR vLook = DirectX::XMLoadFloat3(&look);
        XMVECTOR vUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // World Up

        XMVECTOR vRight = DirectX::XMVector3Cross(vUp, vLook);
        vRight = DirectX::XMVector3Normalize(vRight);

        // Pos = Pos + (Right * Speed)
        XMVECTOR vVelocity = DirectX::XMVectorScale(vRight, speed);
        vPos = DirectX::XMVectorAdd(vPos, vVelocity);

        DirectX::XMStoreFloat3(&pos, vPos);
    }

    void MoveLeft() {
        XMVECTOR vPos = DirectX::XMLoadFloat3(&pos);
        XMVECTOR vLook = DirectX::XMLoadFloat3(&look);
        XMVECTOR vUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        XMVECTOR vRight = DirectX::XMVector3Cross(vUp, vLook);
        vRight = DirectX::XMVector3Normalize(vRight);

        // Pos = Pos - (Right * Speed)
        XMVECTOR vVelocity = DirectX::XMVectorScale(vRight, speed);
        vPos = DirectX::XMVectorSubtract(vPos, vVelocity);

        DirectX::XMStoreFloat3(&pos, vPos);
    }


    void MoveUp() {
        XMVECTOR vPos = DirectX::XMLoadFloat3(&pos);
        XMVECTOR vWorldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        // Pos = Pos + (WorldUp * Speed)
        XMVECTOR vVelocity = DirectX::XMVectorScale(vWorldUp, speed);
        vPos = DirectX::XMVectorAdd(vPos, vVelocity);

        DirectX::XMStoreFloat3(&pos, vPos);
    }


    void MoveDown() {
        XMVECTOR vPos = DirectX::XMLoadFloat3(&pos);
        XMVECTOR vWorldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

        // Pos = Pos - (WorldUp * Speed)
        XMVECTOR vVelocity = DirectX::XMVectorScale(vWorldUp, speed);
        vPos = DirectX::XMVectorSubtract(vPos, vVelocity);

        DirectX::XMStoreFloat3(&pos, vPos);
    }
};
#endif