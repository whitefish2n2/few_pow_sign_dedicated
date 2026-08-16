//
// Created by white on 25. 12. 26..
//

#include "BoxCollider.h"
#include "../GameObjectArgument.h"
#include "../../../../util/StringUtil.h"
#include "../../../Component/Definition/ComponentFactory.h"



REGISTER_COMPONENT(BoxCollider);

Vector3 BoxCollider::CalculateLocalInertia(float mass) const {
    if (gameObject == GameObject::NullPTR()) return Vector3::Zero();

    // 현재 스케일이 반영된 실제 크기(월드 사이즈)를 구합니다.
    const Vector3 scale = gameObject->transform.GetScale();
    Vector3 actualSize = { size.x * scale.x, size.y * scale.y, size.z * scale.z };

    float w2 = actualSize.x * actualSize.x;
    float h2 = actualSize.y * actualSize.y;
    float d2 = actualSize.z * actualSize.z;

    // 직육면체(Box)의 관성 텐서 대각성분 공식 적용
    return {
        (1.0f / 12.0f) * mass * (h2 + d2),
        (1.0f / 12.0f) * mass * (w2 + d2),
        (1.0f / 12.0f) * mass * (w2 + h2)
    };
}

AABB BoxCollider::GetAABB() const {
    if (gameObject == GameObject::NullPTR()) return AABB::Empty();

    const auto& tr = gameObject->transform;
    uint32_t currentPosVer = tr.GetPosVersion();
    uint32_t currentRotVer = tr.GetRotVersion();
    uint32_t currentScaleVer = tr.GetScaleVersion();

    // 1. 크기나 회전, 위치가 바뀐 경우 (AABB 재계산)
    if (transformScaleVersion != currentScaleVer || transformRotVersion != currentRotVer
        || transformPosVersion != currentPosVer) {

        transformRotVersion = currentRotVer;
        transformScaleVersion = currentScaleVer;
        transformPosVersion = currentPosVer;
        lastPosition = tr.GetPosition();

        const Vector3 pos = tr.GetPosition();
        const Vector3 scale = tr.GetScale();
        const Quaternion rot = tr.GetRotation();

        // 박스의 절반 크기(Half Extents)에 스케일을 적용합니다.
        Vector3 halfExtents = { (size.x * scale.x) * 0.5f, (size.y * scale.y) * 0.5f, (size.z * scale.z) * 0.5f };

        // 로컬 Center의 월드 위치 계산
        Vector3 scaledCenter = { center.x * scale.x, center.y * scale.y, center.z * scale.z };
        Vector3 rotatedCenter = rot * scaledCenter;
        Vector3 worldCenter = pos + rotatedCenter;
        
        Matrix4 m = rot.ToMatrix4();

        float ex = std::abs(m.m[0]) * halfExtents.x + std::abs(m.m[1]) * halfExtents.y + std::abs(m.m[2]) * halfExtents.z;
        float ey = std::abs(m.m[4]) * halfExtents.x + std::abs(m.m[5]) * halfExtents.y + std::abs(m.m[6]) * halfExtents.z;
        float ez = std::abs(m.m[8]) * halfExtents.x + std::abs(m.m[9]) * halfExtents.y + std::abs(m.m[10]) * halfExtents.z;

        Vector3 extentResult = { ex, ey, ez };

        cachedAABB.min = worldCenter - extentResult;
        cachedAABB.max = worldCenter + extentResult;
    }
    // 2. 위치만 바뀐 경우 (초고속 이동 처리)
    else if (transformPosVersion != currentPosVer) {
        Vector3 currentPos = tr.GetPosition();
        Vector3 delta = currentPos - lastPosition;

        cachedAABB.min += delta;
        cachedAABB.max += delta;

        transformPosVersion = currentPosVer;
        lastPosition = currentPos;
    }

    return cachedAABB;
}

void BoxCollider::ParseFromString(const std::string& arg) {
    std::stringstream ss(arg);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        size_t delimPos = line.find(':');
        if (delimPos == std::string::npos) continue;
        std::string key = StringUtils::Trim(line.substr(0, delimPos));
        std::string val = StringUtils::Trim(line.substr(delimPos + 1));

        if (key == "IsTrigger") {
            this->isTrigger = (val == "1");
        }
        else if (key == "Center") {
            this->center = Vector3::ParseVector3(val);
        }
        else if (key == "Size") {
            this->size =Vector3::ParseVector3(val);
        }
        else if (key == "StaticFriction") {
            this->material.staticFriction = std::stof(val);
        }
        else if (key == "DynamicFriction") {
            this->material.dynamicFriction = std::stof(val);
        }
        else if (key == "Bounciness") {
            this->material.bounciness = std::stof(val);
        }
        else if (key == "BounceCombine") {
            this->material.bounceCombine = ColliderMaterial::ParseCombineMode(val);
        }
        else if (key == "FrictionCombine") {
            this->material.frictionCombine = ColliderMaterial::ParseCombineMode(val);
        }
    }
}
#ifdef _WIN64
#include "../../Renderer.h"
Renderer BoxCollider::GetRenderer() {
    Renderer r{};

    if (this->gameObject == GameObject::NullPTR() || this->gameObject->id == (uint64_t)-1) {
        return r; // 유령 객체면 렌더러 생성 중지!
    }

    r.owner = this->gameObject;
    //std::cout << "owner setting: " << this->gameObject->name << std::endl;

    r.mesh = MeshManager::GetInstance()->GetUnitBox();

    if (r.mesh) {
        r.color = { 0.0f, 1.0f, 0.0f, 1.0f };
        r.isWireframe = true;


        r.localScale = reinterpret_cast<const DirectX::XMFLOAT3&>(this->size);

        r.localOffset = reinterpret_cast<const DirectX::XMFLOAT3&>(this->center);
    }
    else {
        r = Renderer::ErrorRenderer(this->gameObject,&this->gameObject->transform);
    }

    return r;
}
#endif
