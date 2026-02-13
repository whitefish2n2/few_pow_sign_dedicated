//
// Created by white on 26. 2. 12..
//

#ifndef FPSPROJECTSERVER_MESHMANAGER_H
#define FPSPROJECTSERVER_MESHMANAGER_H
#ifdef _WIN64
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "Mesh.h"
#include "../../SessionDXViewer/DirectXCore.h"
#include "../vector/Vector3.h"

struct SimpleVertex {
    SimpleVertex(Vector3 v, DirectX::XMFLOAT4 color):pos(reinterpret_cast<DirectX::XMFLOAT3&>(v)), color(color) {}
    SimpleVertex(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 color):pos(pos), color(color) {}
    SimpleVertex() = default;
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT4 color;
};
struct Mesh;

class MeshManager {
    std::map<std::string, Mesh*> meshCache{};
    std::mutex meshCacheMutex;
    MeshManager() = default;
    ~MeshManager() {
        for (auto& pair : meshCache) {
            if (pair.second) {
                delete pair.second;
            }
        }
        meshCache.clear();
    }
    public:
    MeshManager(const MeshManager&) = delete;
    MeshManager& operator=(const MeshManager&) = delete;
    static MeshManager* GetInstance() {
        static MeshManager instance;
        return &instance;
    }

    Mesh* GetMesh(const std::string& name) {
        std::lock_guard<std::mutex> lock(meshCacheMutex);
        auto it = meshCache.find(name);
        if (it != meshCache.end()) {
            return it->second;
        }
        return nullptr;
    }

    ///기본 메쉬 획득 함수. 메쉬 이름이 "UnitCube","UnitCircle","UnitCapsule"일 경우 충돌 발생 가능성 있으니 조심
    Mesh* GetUnitBox() {
        if (auto it = GetMesh("UnitCube"); it != nullptr) return it;

        std::vector<SimpleVertex> v = {
            { DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(1,0,0,1) },
            { DirectX::XMFLOAT3( 0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(0,1,0,1) },
            { DirectX::XMFLOAT3( 0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(0,0,1,1) },
            { DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(1,1,0,1) },
            { DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(0,1,1,1) },
            { DirectX::XMFLOAT3( 0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(1,0,1,1) },
            { DirectX::XMFLOAT3( 0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(0,0,0,1) },
            { DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(1,1,1,1) },
        };
        std::vector<uint32_t> i = {
            3,1,0, 2,1,3, // 윗면
            0,5,4, 1,5,0, // 앞면
            3,4,7, 0,4,3, // 왼쪽
            1,6,5, 2,6,1, // 오른쪽
            2,7,6, 3,7,2, // 뒷면
            6,4,5, 7,4,6  // 아랫면
        };

        GetOrCreateMesh("UnitCube", v, i);
        return GetMesh("UnitCube");
    }
    Mesh* GetUnitSphere() {
        if (auto it = GetMesh("UnitSphere"); it != nullptr) return it;

        std::vector<SimpleVertex> v;
        std::vector<uint32_t> i;

        float radius = 0.5f;
        int stackCount = 20; // 가로 줄
        int sliceCount = 20; // 세로 줄

        // 구 만들기 알고리즘
        float phiStep = DirectX::XM_PI / stackCount;
        float thetaStep = 2.0f * DirectX::XM_PI / sliceCount;

        for (int x = 0; x <= stackCount; x++) {
            float phi = x * phiStep;
            for (int y = 0; y <= sliceCount; y++) {
                float theta = y * thetaStep;
                SimpleVertex vert{};
                vert.pos.x = radius * sinf(phi) * cosf(theta);
                vert.pos.y = radius * cosf(phi);
                vert.pos.z = radius * sinf(phi) * sinf(theta);
                vert.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // 흰색
                v.push_back(vert);
            }
        }

        for (int x = 0; x < stackCount; x++) {
            for (int y = 0; y < sliceCount; y++) {
                WORD topLeft = (sliceCount + 1) * x + y;
                WORD topRight = topLeft + 1;
                WORD bottomLeft = (sliceCount + 1) * (x + 1) + y;
                WORD bottomRight = bottomLeft + 1;

                i.push_back(topLeft); i.push_back(topRight); i.push_back(bottomLeft);
                i.push_back(bottomLeft); i.push_back(topRight); i.push_back(bottomRight);
            }
        }

        GetOrCreateMesh("UnitSphere", v, i);
        return GetMesh("UnitSphere");
    }

    Mesh* GetUnitCapsule() {
        if (auto it = GetMesh("UnitCapsule"); it != nullptr) return it;

        std::vector<SimpleVertex> v;
        std::vector<uint32_t> i;

        float radius = 0.5f;
        float height = 1.0f; // 실린더 부분의 높이 (총 높이 2.0 - 위아래 캡 1.0)
        float halfHeight = height * 0.5f;

        int stackCount = 20; // 반구당 가로 줄 수 (정밀도)
        int sliceCount = 20; // 세로 줄 수

        float phiStep = DirectX::XM_PIDIV2 / stackCount; // 90도를 stackCount로 나눔
        float thetaStep = 2.0f * DirectX::XM_PI / sliceCount;

        // 1. Top Hemisphere (위쪽 뚜껑)
        // Phi: 0 ~ 90도
        for (int x = 0; x <= stackCount; x++) {
            float phi = x * phiStep;
            for (int y = 0; y <= sliceCount; y++) {
                float theta = y * thetaStep;
                SimpleVertex vert{};
                vert.pos.x = radius * sinf(phi) * cosf(theta);
                vert.pos.y = radius * cosf(phi) + halfHeight; // 위로 올림
                vert.pos.z = radius * sinf(phi) * sinf(theta);
                vert.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
                v.push_back(vert);
            }
        }

        // 2. Bottom Hemisphere (아래쪽 뚜껑)
        // Phi: 90 ~ 180도.
        // 주의: 위쪽 루프의 마지막 줄(y=0.5)과 아래쪽 루프의 첫 줄(y=-0.5) 사이가 원기둥이 됨
        for (int x = 0; x <= stackCount; x++) {
            float phi = DirectX::XM_PIDIV2 + (x * phiStep); // 90도부터 시작
            for (int y = 0; y <= sliceCount; y++) {
                float theta = y * thetaStep;
                SimpleVertex vert{};
                vert.pos.x = radius * sinf(phi) * cosf(theta);
                vert.pos.y = radius * cosf(phi) - halfHeight; // 아래로 내림
                vert.pos.z = radius * sinf(phi) * sinf(theta);
                vert.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
                v.push_back(vert);
            }
        }

        // 3. 인덱스 생성
        // 총 Stack 수 = (위쪽 stack) + (중간 원기둥 연결부 1개) + (아래쪽 stack)
        // 위쪽 루프 0~stackCount, 아래쪽 루프 0~stackCount 이므로
        // 전체 줄(Row) 개수는 (stackCount + 1) * 2
        // 전체 면(Stack) 개수는 TotalRows - 1
        int totalStacks = (stackCount * 2) + 1;

        for (int x = 0; x < totalStacks; x++) {
            for (int y = 0; y < sliceCount; y++) {
                WORD topLeft = (sliceCount + 1) * x + y;
                WORD topRight = topLeft + 1;
                WORD bottomLeft = (sliceCount + 1) * (x + 1) + y;
                WORD bottomRight = bottomLeft + 1;

                i.push_back(topLeft); i.push_back(topRight); i.push_back(bottomLeft);
                i.push_back(bottomLeft); i.push_back(topRight); i.push_back(bottomRight);
            }
        }

        GetOrCreateMesh("UnitCapsule", v, i);
        return GetMesh("UnitCapsule");
    }

     Mesh* GetOrCreateMesh(const std::string& meshName, const std::vector<SimpleVertex>& vertices, const std::vector<uint32_t>& indices) {
        std::lock_guard<std::mutex> lock(meshCacheMutex);
        if (meshCache.find(meshName) != meshCache.end()) {
            return meshCache[meshName];
        }
        Mesh* newMesh = new Mesh();

        // --- 버텍스 버퍼 생성 ---
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(SimpleVertex) * vertices.size();
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = vertices.data();

        HRESULT hr = DirectXCore::device->CreateBuffer(&bd, &initData, &newMesh->vertexBuffer);
        if (FAILED(hr)) {
            delete newMesh;
            return nullptr;
        }

        // --- 인덱스 버퍼 생성 ---
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(WORD) * indices.size();
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;

        initData.pSysMem = indices.data();

        hr = DirectXCore::device->CreateBuffer(&bd, &initData, &newMesh->indexBuffer);
        if (FAILED(hr)) {
            newMesh->vertexBuffer->Release();
            delete newMesh;
            return nullptr;
        }

        // --- 메타 데이터 저장 ---
        newMesh->vertexStride = sizeof(SimpleVertex);
        newMesh->indexCount = static_cast<uint32_t>(indices.size());

        //캐시에 등록
        meshCache[meshName] = newMesh;
         return meshCache[meshName];
    }
};
#endif
#endif //FPSPROJECTSERVER_MESHMANAGER_H