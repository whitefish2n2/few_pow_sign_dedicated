//
// Created by white on 25. 12. 23..
//
#include "MeshCollider.h"

#include "../GameObject.h"
#include "../../quaternion/Quaternion.h"
#include "../GameObjectArgument.h"
#include "../../../Component/Definition/ComponentFactory.h"


AABB MeshCollider::GetAABB() const {
    const auto& verts = GetVertices();
    AABB aabb = AABB::Empty();
    aabb.min = verts[0];
    aabb.max = verts[0];
    const Vector3 pos   =  gameObject-> transform.position;
    const Vector3 scale = gameObject->transform.scale;
    const Quaternion rot = gameObject->transform.rotation;


    for (size_t i = 1; i < verts.size(); ++i) {
        const Vector3& v = verts[i];

        if (v.x < aabb.min.x) aabb.min.x = v.x;
        if (v.y < aabb.min.y) aabb.min.y = v.y;
        if (v.z < aabb.min.z) aabb.min.z = v.z;

        if (v.x > aabb.max.x) aabb.max.x = v.x;
        if (v.y > aabb.max.y) aabb.max.y = v.y;
        if (v.z > aabb.max.z) aabb.max.z = v.z;
    }

    return aabb;
}

void MeshCollider::ParseFromString(const std::string& arg) {
    std::stringstream ss(arg);
    std::string line;

    // 파싱 상태 (0: 기본, 1: 버텍스 읽기, 2: 삼각형 읽기)
    int mode = 0;
    int countToRead = 0;

    this->vertices.clear();
    this->triangles.clear();

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        // 1. 헤더(Key: Value) 확인
        size_t delimPos = line.find(": ");
        if (delimPos != std::string::npos) {
            std::string key = line.substr(0, delimPos);
            std::string val = line.substr(delimPos + 2);

            if (key == "IsTrigger") {
                this->isTrigger = (val == "1");
                mode = 0;
            }
            else if (key == "VertexCount") {
                countToRead = std::stoi(val);
                this->vertices.reserve(countToRead);
                mode = 1;
            }
            else if (key == "TriangleCount") {
                countToRead = std::stoi(val);
                this->triangles.reserve(countToRead);
                mode = 2;
            }
            continue;
        }

        if (mode == 1 && countToRead > 0) { // Vertex Read Mode
            // 포맷: "x,y,z"
            this->vertices.push_back(Vector3::ParseVector3(line));
            countToRead--;
        }
        else if (mode == 2 && countToRead > 0) { // Triangle Read Mode
            // 포맷: "idx1,idx2,idx3"
            int i1, i2, i3;
            if (sscanf_s(line.c_str(), "%d,%d,%d", &i1, &i2, &i3) == 3) {
                if (i1 < vertices.size() && i2 < vertices.size() && i3 < vertices.size()) {
                    this->triangles.emplace_back(Triangle(i1, i2, i3));
                }
            }
            countToRead--;
        }
    }
    CalculateAABB();
}

#ifdef _WIN64
#include "../../Renderer.h"
#include "../../Mesh/MeshManager.h"
Renderer MeshCollider::GetRenderer() {
    Renderer r{};

    if (this->gameObject!=GameObject::NullPTR())
        r.mesh = MeshManager::GetInstance()->GetMesh(this->gameObject->name);
    else return r;

    if (r.mesh == nullptr) {
        std::vector<SimpleVertex> verts;
        for (auto v: this->vertices) {
            verts.push_back(SimpleVertex(v,));
        }
        r = MeshManager::GetOrCreateMesh(this->gameObject->name, );
        r.mesh = MeshManager::GetInstance()->GetUnitBox();
        r.color = { 1.0f, 0.0f, 0.0f, 1.0f }; // 빨강
        r.localScale = { 1.0f, 1.0f, 1.0f };
    }
    else {
        // 정상적으로 찾음
        r.color = { 0.0f, 0.0f, 1.0f, 1.0f }; // 파랑 (메쉬 콜라이더 구분용)
        r.localScale = reinterpret_cast<DirectX::XMFLOAT3&>(this->gameObject->transform.scale);
    }
}
#endif

REGISTER_COMPONENT(MeshCollider)
