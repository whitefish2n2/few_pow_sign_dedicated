//
// Created by white on 26. 2. 12..
//

#include "SphereCollider.h"

#include <complex>
#include <sstream>

#include "../GameObjectArgument.h"
#include "../../../Component/Definition/ComponentFactory.h"
#include <algorithm>

#include "../../../../util/StringUtil.h"

AABB SphereCollider::GetAABB() const {
    if (gameObject == GameObject::NullPTR()) return AABB::Empty();

    const auto& tr = gameObject->transform;
    uint32_t currentPosVer = tr.GetPosVersion();
    uint32_t currentRotVer = tr.GetRotVersion();
    uint32_t currentScaleVer = tr.GetScaleVersion();

    // 1. 크기(Scale)나 회전(Rotation)이 변경된 경우
    // 구체 자체는 회전해도 크기가 같지만, 로컬 center가 (0,0,0)이 아닐 경우 위치가 변하므로 회전도 체크합니다.
    if (transformScaleVersion != currentScaleVer || transformRotVersion != currentRotVer) {

        transformRotVersion = currentRotVer;
        transformScaleVersion = currentScaleVer;
        transformPosVersion = currentPosVer;
        lastPosition = tr.GetPosition();

        const Vector3 pos = tr.GetPosition();
        const Vector3 scale = tr.GetScale();
        const Quaternion rot = tr.GetRotation();

        // 🌟 핵심 타협점: X, Y, Z 중 가장 큰 스케일을 찾아 완벽한 구의 반지름으로 사용
        float maxScale = (std::max)({scale.x, scale.y, scale.z});
        float scaledRadius = this->radius * maxScale;

        // 로컬 Center의 월드 위치 계산 (Center 오프셋에 회전과 스케일 적용)
        Vector3 scaledCenter = { center.x * scale.x, center.y * scale.y, center.z * scale.z };
        Vector3 rotatedCenter = rot * scaledCenter;
        Vector3 worldCenter = pos + rotatedCenter;

        // 타원체 Extent 공식이 사라지고, 단순히 반지름을 빼고 더하는 것으로 끝납니다!
        Vector3 extents = { scaledRadius, scaledRadius, scaledRadius };
        cachedAABB.min = worldCenter - extents;
        cachedAABB.max = worldCenter + extents;
    }
    // 2. 크기/회전은 그대로고 위치(Position)만 변경된 경우 (초고속 이동)
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

Vector3 SphereCollider::CalculateLocalInertia(float mass) const {
    if (gameObject == GameObject::NullPTR()) return Vector3::Zero();

    const Vector3 scale = gameObject->transform.GetScale();
    float maxScale = (std::max)({scale.x, scale.y, scale.z});
    float scaledRadius = this->radius * maxScale;

    float i = (2.0f / 5.0f) * mass * (scaledRadius * scaledRadius);
    
    return { i, i, i };
}

void SphereCollider::ParseFromString(const std::string &arg) {
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
        else if (key == "Radius") {
            this->radius = std::stof(val);
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
Renderer SphereCollider::GetRenderer() {
    Renderer r{};

    if (this->gameObject == GameObject::NullPTR()) return r;

    r.owner = this->gameObject;

    r.mesh = MeshManager::GetInstance()->GetUnitSphere();

    if (r.mesh) {
        r.color = { 0.0f, 1.0f, 0.0f, 1.0f };
        r.isWireframe = true;

        float scaleFactor = this->radius * 2.0f;

        r.localScale = { scaleFactor, scaleFactor, scaleFactor };


        r.localOffset = reinterpret_cast<const DirectX::XMFLOAT3&>(this->center);
    }
    else {
        r = Renderer::ErrorRenderer(this->gameObject,&this->gameObject->transform);
    }

    return r;
}
#endif

REGISTER_COMPONENT(SphereCollider);