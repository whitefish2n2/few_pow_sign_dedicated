//
// Created by white on 26. 2. 12..
//
#pragma once
#ifdef _WIN64
#ifndef FPSPROJECTSERVER_RENDERER_H
#define FPSPROJECTSERVER_RENDERER_H
#include "Mesh/MeshManager.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <d3d11.h>
#include <dxgiformat.h>

#include "Mesh/Mesh.h"


class Transform;

class Renderer {
public:
    Renderer() = default;
    ~Renderer() = default;


    ///false일시 렌더링하지않음
    bool enable = true;
    GameObject owner = GameObject::NullPTR();
    Transform* transform = nullptr; // 위치 정보
    Mesh* mesh;
    bool isWireframe = false;

    DirectX::XMFLOAT3 localScale = { 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 localOffset = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT4 color = { 0.0f, 1.0f, 0.0f, 1.0f };

    void Draw(ID3D11DeviceContext* context) {
        if (!mesh || !mesh->vertexBuffer) return;

        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, &mesh->vertexStride, &offset);
        context->IASetIndexBuffer(mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        context->DrawIndexed(mesh->indexCount, 0, 0);
    }
    static Renderer ErrorRenderer(GameObject owner, Transform* transform) {
        Renderer r;

        // 1. 모양: 가장 만만한 정육면체
        r.mesh = MeshManager::GetInstance()->GetUnitBox() ;

        // 2. 색상: 눈이 아플 정도로 쨍한 마젠타색 (R:1, G:0, B:1)
        r.color = { 1.0f, 0.0f, 1.0f, 1.0f };

        // 3. 크기: 잘 보이게 기본 1배
        r.localScale = { 1.0f, 1.0f, 1.0f };
        r.localOffset = { 0.0f, 0.0f, 0.0f };
        r.transform = transform;
        r.owner = owner;
        return r;
    }

};
#endif //FPSPROJECTSERVER_RENDERER_H
#endif