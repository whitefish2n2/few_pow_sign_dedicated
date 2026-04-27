//
// Created by white on 25. 12. 23..
//
#include "MeshCollider.h"

#include "ColliderMaterial.h"
#include "../GameObject.h"
#include "../../quaternion/Quaternion.h"
#include "../GameObjectArgument.h"
#include "../../../../util/StringUtil.h"
#include "../../../Component/Definition/ComponentFactory.h"


Vector3 MeshCollider::CalculateLocalInertia(float mass) const {
    if (this->staticObject) return Vector3::Zero();

    // AABB를 구해옵니다.
    AABB bounds = GetAABB();

    // AABB의 가로, 세로, 깊이 길이를 구합니다 (max - min)
    float width = bounds.max.x - bounds.min.x;
    float height = bounds.max.y - bounds.min.y;
    float depth = bounds.max.z - bounds.min.z;

    // 직육면체(Box)의 관성 텐서 공식으로 근사하여 반환합니다.
    return {
        (1.0f / 12.0f) * mass * (height * height + depth * depth),
        (1.0f / 12.0f) * mass * (width * width + depth * depth),
        (1.0f / 12.0f) * mass * (width * width + height * height)
    };
}

AABB MeshCollider::GetAABB() const {
    // 1. 예외 처리: 로컬 AABB가 아직 계산되지 않았다면 (혹은 비어있다면) 다시 계산
    if (!isLocalAABBCalculated) {
        CalculateAABB();
    }

    // 정점이 아예 없는 메쉬라면 빈 AABB 반환
    if (vertices.empty()) {
        return localAABB;
    }
    auto transform = gameObject->transform;
    if (transformScaleVersion != transform.GetScaleVersion() || transformRotVersion != transform.GetRotVersion()) {
        transformRotVersion = transform.GetRotVersion();
        transformScaleVersion = transform.GetScaleVersion();
        transformPosVersion = transform.GetPosVersion();
        lastPosition = transform.GetPosition();
        const Vector3 pos = transform.GetPosition();
        const Vector3 scale = transform.GetScale();
        const Quaternion rot = transform.GetRotation();

        // 2. 로컬 AABB의 8개 모서리(코너) 좌표 추출
        Vector3 corners[8] = {
            {localAABB.min.x, localAABB.min.y, localAABB.min.z},
            {localAABB.max.x, localAABB.min.y, localAABB.min.z},
            {localAABB.min.x, localAABB.max.y, localAABB.min.z},
            {localAABB.max.x, localAABB.max.y, localAABB.min.z},
            {localAABB.min.x, localAABB.min.y, localAABB.max.z},
            {localAABB.max.x, localAABB.min.y, localAABB.max.z},
            {localAABB.min.x, localAABB.max.y, localAABB.max.z},
            {localAABB.max.x, localAABB.max.y, localAABB.max.z}
        };

        AABB worldAABB = AABB::Empty();

        // 3. 8개의 코너에만 Transform(Scale -> Rotation -> Position) 적용하여 월드 AABB 도출
        for (int i = 0; i < 8; ++i) {
            Vector3 scaled = { corners[i].x * scale.x, corners[i].y * scale.y, corners[i].z * scale.z };
            Vector3 rotated = rot * scaled;
            Vector3 worldPos = rotated + pos;

            // 첫 번째 코너로 월드 AABB 초기화
            if (i == 0) {
                worldAABB.min = worldPos;
                worldAABB.max = worldPos;
            } else {
                // 나머지 7개 코너로 Min/Max 갱신
                if (worldPos.x < worldAABB.min.x) worldAABB.min.x = worldPos.x;
                if (worldPos.y < worldAABB.min.y) worldAABB.min.y = worldPos.y;
                if (worldPos.z < worldAABB.min.z) worldAABB.min.z = worldPos.z;

                if (worldPos.x > worldAABB.max.x) worldAABB.max.x = worldPos.x;
                if (worldPos.y > worldAABB.max.y) worldAABB.max.y = worldPos.y;
                if (worldPos.z > worldAABB.max.z) worldAABB.max.z = worldPos.z;
            }
        }
        cachedAABB = worldAABB;
    }
    else if (transformPosVersion != transform.GetPosVersion()) {
        Vector3 currentPos = transform.GetPosition();
        Vector3 delta = currentPos - lastPosition;

        // 단순히 이동량(Delta)만큼 AABB를 옮겨줌
        cachedAABB.min += delta;
        cachedAABB.max += delta;

        transformPosVersion = transform.GetPosVersion();
        lastPosition = currentPos;
    }


    return cachedAABB;
}

void MeshCollider::CalculateAABB() const {
    const auto& verts = GetVertices();

    // 정점이 없는 경우 처리
    if (verts.empty()) {
        localAABB = AABB::Empty();
        isLocalAABBCalculated = true;
        return;
    }

    localAABB.min = verts[0];
    localAABB.max = verts[0];

    // 로컬 정점만으로 Min/Max 계산
    for (size_t i = 1; i < verts.size(); ++i) {
        const Vector3& v = verts[i];

        if (v.x < localAABB.min.x) localAABB.min.x = v.x;
        if (v.y < localAABB.min.y) localAABB.min.y = v.y;
        if (v.z < localAABB.min.z) localAABB.min.z = v.z;

        if (v.x > localAABB.max.x) localAABB.max.x = v.x;
        if (v.y > localAABB.max.y) localAABB.max.y = v.y;
        if (v.z > localAABB.max.z) localAABB.max.z = v.z;
    }

    isLocalAABBCalculated = true; // 계산 완료 플래그 켜기
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

        size_t delimPos = line.find(':');
        if (delimPos == std::string::npos) continue;
        std::string key = StringUtils::Trim(line.substr(0, delimPos));
        std::string val = StringUtils::Trim(line.substr(delimPos + 1));
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
        else if (key == "StaticFriction") {
            this->material.staticFriction = std::stof(val);
            mode = 0;
        }
        else if (key == "DynamicFriction") {
            this->material.dynamicFriction = std::stof(val);
            mode = 0;
        }
        else if (key == "Bounciness") {
            this->material.bounciness = std::stof(val);
            mode = 0;
        }
        else if (key == "BounceCombine") {
            this->material.bounceCombine = ColliderMaterial::ParseCombineMode(val);
            mode = 0;
        }
        else if (key == "FrictionCombine") {
            this->material.frictionCombine = ColliderMaterial::ParseCombineMode(val);
            mode = 0;
        }
        continue;

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
