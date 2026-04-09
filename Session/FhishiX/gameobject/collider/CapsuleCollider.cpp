#include "CapsuleCollider.h"
#include <algorithm>
#include <cmath>

#include "../../../Component/Definition/ComponentFactory.h"
using namespace std;
#include "../GameObjectArgument.h"
//
// Created by white on 25. 12. 23..
//
AABB CapsuleCollider::GetAABB() const {
    AABB aabb = AABB::Empty();

    const auto& tr = gameObject->transform;


    // 1) 로컬 파라미터
    const float halfH_local = height * 0.5f;

    // 2) 스케일 적용
    const float scaledRadius = radius * max(tr.GetScale().x, tr.GetScale().z);
    const float scaledHalfH  = halfH_local * tr.GetScale().y;

    // 3) 로컬 top / bottom
    Vector3 localTop    = center + Vector3(0, +scaledHalfH, 0);
    Vector3 localBottom = center + Vector3(0, -scaledHalfH, 0);

    // 4) 회전 및 위치 적용
    Vector3 worldTop    = tr.GetRotation() * localTop    + tr.GetPosition();
    Vector3 worldBottom = tr.GetRotation() * localBottom + tr.GetPosition();

    // 5) 두 점으로 AABB 만들고 radius 확장
    aabb.min.x = min(worldTop.x, worldBottom.x) - scaledRadius;
    aabb.min.y = min(worldTop.y, worldBottom.y) - scaledRadius;
    aabb.min.z = min(worldTop.z, worldBottom.z) - scaledRadius;

    aabb.max.x = max(worldTop.x, worldBottom.x) + scaledRadius;
    aabb.max.y = max(worldTop.y, worldBottom.y) + scaledRadius;
    aabb.max.z = max(worldTop.z, worldBottom.z) + scaledRadius;

    return aabb;
}

void CapsuleCollider::ParseFromString(const std::string &arg) {
    std::stringstream ss(arg);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        size_t delimPos = line.find(": ");
        if (delimPos == std::string::npos) continue;

        std::string key = line.substr(0, delimPos);
        std::string val = line.substr(delimPos + 2);

        if (key == "IsTrigger") {
            this->isTrigger = (val == "1");
        }
        else if (key == "Center") {
            this->center =Vector3::ParseVector3(val);
        }
        else if (key == "Radius") {
            this->radius = std::stof(val);
        }
        else if (key == "Height") {
            this->height = std::stof(val);
        }
        else if (key == "Direction") {
            this->direction = std::stoi(val); // 0:X, 1:Y, 2:Z (Unity 기준)
        }
    }

}
#ifdef _WIN64
#include "../../Renderer.h"
Renderer CapsuleCollider::GetRenderer() {
    Renderer r{};
    if (this->gameObject == GameObject::NullPTR() || this->gameObject->id == (uint64_t)-1) {
        return r;
    }
    if (this->gameObject == GameObject::NullPTR()) return r;

    r.owner = this->gameObject;

    r.mesh = MeshManager::GetInstance()->GetUnitCapsule();

    if (r.mesh) {
        r.color = { 0.0f, 1.0f, 0.0f, 1.0f };
        r.isWireframe = true;

        // UnitCapsule: 반지름 0.5 (지름 1.0), 높이 2.0

        // 1. 가로/세로(XZ) 스케일: 반지름(radius)을 맞추려면 * 2.0 필요
        float diameterScale = this->radius * 2.0f;

        // 2. 높이(Y) 스케일: Unit 높이가 2.0이므로, 원하는 높이(height)가 되려면 / 2.0 필요
        float heightScale = this->height / 2.0f;

        r.localScale = { diameterScale, heightScale, diameterScale };

        // 오프셋
        r.localOffset = reinterpret_cast<const DirectX::XMFLOAT3&>(this->center);
    }
    else {
        r = Renderer::ErrorRenderer(this->gameObject,&this->gameObject->transform);
    }

    return r;
}
#endif

REGISTER_COMPONENT(CapsuleCollider);
