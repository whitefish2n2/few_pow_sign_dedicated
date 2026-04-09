//
// Created by white on 26. 2. 12..
//

#include "SphereCollider.h"

#include <complex>
#include <sstream>

#include "../GameObjectArgument.h"
#include "../../../Component/Definition/ComponentFactory.h"
AABB SphereCollider::GetAABB() const {
    AABB aabb = AABB::Empty();
    if (gameObject == GameObject::NullPTR()) return aabb;

    const auto& tr = gameObject->transform;

    // 1. 구의 스케일은 X, Y, Z 중 가장 큰 값을 기준으로 잡습니다.
    float maxScale = (std::max)({tr.GetScale().x, tr.GetScale().y, tr.GetScale().z});
    float scaledRadius = this->radius * maxScale;

    // 2. 월드 좌표 기준 중심점 계산 (Transform 위치 + 로컬 Center 오프셋)
    Vector3 worldCenter = tr.GetPosition() + this->center;

    // 3. 중심점을 기준으로 반지름만큼 빼고 더해서 박스(AABB)를 만듭니다.
    aabb.min.x = worldCenter.x - scaledRadius;
    aabb.min.y = worldCenter.y - scaledRadius;
    aabb.min.z = worldCenter.z - scaledRadius;

    aabb.max.x = worldCenter.x + scaledRadius;
    aabb.max.y = worldCenter.y + scaledRadius;
    aabb.max.z = worldCenter.z + scaledRadius;

    return aabb;
}
void SphereCollider::ParseFromString(const std::string &arg) {
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
            this->center = Vector3::ParseVector3(val);
        }
        else if (key == "Radius") {
            this->radius = std::stof(val);
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