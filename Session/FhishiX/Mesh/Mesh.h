//
// Created by white on 26. 2. 12..
//

#ifndef FPSPROJECTSERVER_MESH_H
#define FPSPROJECTSERVER_MESH_H
#ifdef _WIN64
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

struct Mesh {

    std::vector<DirectX::XMFLOAT3> vertices{};
    std::vector<uint16_t> indices{};

    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;

    UINT vertexStride = 0;
    UINT indexCount = 0;

    ~Mesh() {
        if (vertexBuffer) vertexBuffer->Release();
        if (indexBuffer) indexBuffer->Release();
    }
};
#endif
#endif //FPSPROJECTSERVER_MESH_H