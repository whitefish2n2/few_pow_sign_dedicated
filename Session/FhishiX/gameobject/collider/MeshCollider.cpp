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
    const Vector3 pos   =  gameObject-> transform.GetPosition();
    const Vector3 scale = gameObject->transform.GetScale();
    const Quaternion rot = gameObject->transform.GetRotation();


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

    int mode = 0;
    int countToRead = 0;

    this->vertices.clear();
    this->triangles.clear();
    this->trianglesIndices.clear();

    // ⬇️ 1. 데이터가 통째로 잘 넘어왔는지 확인!
    std::cout << "\n--- [MeshCollider 파싱 시작: " << this->gameObject->name << "] ---\n";
    std::cout << "받은 전체 문자열 길이: " << arg.length() << " bytes\n";

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

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
                std::cout << ">> 정점 모드 진입 (읽을 개수: " << countToRead << ")\n"; // ⬇️ 2. 모드 진입 확인
            }
            else if (key == "TriangleCount") {
                countToRead = std::stoi(val);
                this->triangles.reserve(countToRead);
                mode = 2;
                std::cout << ">> 삼각형 모드 진입 (읽을 개수: " << countToRead << ")\n"; // ⬇️ 3. 모드 진입 확인
            }
            continue;
        }

        if (mode == 1 && countToRead > 0) {
            try {
                this->vertices.push_back(Vector3::ParseVector3(line));
                countToRead--;
            } catch (...) {
                std::cout << "💥 정점 파싱 에러 발생 라인: " << line << "\n";
            }
        }
        else if (mode == 2 && countToRead > 0) {
            uint32_t i1, i2, i3;
            if (sscanf_s(line.c_str(), "%u,%u,%u", &i1, &i2, &i3) == 3) {
                if (i1 < vertices.size() && i2 < vertices.size() && i3 < vertices.size()) {
                    this->triangles.emplace_back(i1, i2, i3);
                    trianglesIndices.push_back(i1);
                    trianglesIndices.push_back(i2);
                    trianglesIndices.push_back(i3);
                } else {
                    std::cout << "💥 인덱스 범위 초과 라인: " << line << "\n";
                }
            } else {
                std::cout << "💥 인덱스 스캔 실패 라인: " << line << "\n";
            }
            countToRead--;
        }
    }

    // ⬇️ 4. 최종 결과 확인
    std::cout << "최종 파싱 완료 -> 정점: " << this->vertices.size()
              << "개, 인덱스: " << this->trianglesIndices.size() << "개\n";
    std::cout << "--------------------------------------\n\n";
}

    #ifdef _WIN64
    #include "../../Renderer.h"
    #include "../../Mesh/MeshManager.h"
Renderer MeshCollider::GetRenderer() {
    Renderer r{};

    // 1. 유령 객체 방어벽 (BoxCollider와 동일하게 적용)
    if (this->gameObject == GameObject::NullPTR() || this->gameObject->id == (uint64_t)-1) {
        return r;
    }


    r.owner = this->gameObject;
    std::cout << "MeshCollider GetRenderer Call: " << this->gameObject->name << std::endl;


    r.mesh = MeshManager::GetInstance()->GetMesh(this->gameObject->name);


    if (r.mesh == nullptr) {
        if (this->vertices.empty() || this->trianglesIndices.empty()) {
            std::cout << "[Warning] " << this->gameObject->name << "의 정점이나 인덱스 데이터가 비어있습니다!" << std::endl;
            return Renderer::ErrorRenderer(this->gameObject, &this->gameObject->transform);
        }

        std::vector<SimpleVertex> verts;
        verts.reserve(this->vertices.size()); // 메모리 할당 최적화
        for (auto v : this->vertices) {

            verts.push_back(SimpleVertex(v, { 0.8f, 0.8f, 0.8f, 1.0f }));
        }


        r.mesh = MeshManager::GetInstance()->GetOrCreateMesh(this->gameObject->name, verts, trianglesIndices);
    }


    if (r.mesh != nullptr) {
        r.color = { 0.0f, 0.0f, 1.0f, 1.0f }; // 파란색
        r.isWireframe = true;

        Vector3 scale = gameObject->transform.GetScale();
        r.localScale = { 1,1,1 };
    }
    else {
        std::cout << "[Error] DX Buffer 생성 실패: " << this->gameObject->name << std::endl;
        r = Renderer::ErrorRenderer(this->gameObject, &this->gameObject->transform);
    }

    return r;
}
#endif

REGISTER_COMPONENT(MeshCollider)
