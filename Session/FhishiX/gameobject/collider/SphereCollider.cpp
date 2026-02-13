//
// Created by white on 26. 2. 12..
//

#include "SphereCollider.h"

#include <sstream>

#include "../GameObjectArgument.h"

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

    CalculateAABB();
}
#ifdef _WIN64
#include "../../Renderer.h"
Renderer SphereCollider::GetRenderer() {
    Renderer r{};

    if (this->gameObject == GameObject::NullPTR()) return r;

    r.owner = this->gameObject;
    r.transform = &this->gameObject->transform;

    r.mesh = MeshManager::GetInstance()->GetUnitSphere();

    if (r.mesh) {
        r.color = { 0.0f, 1.0f, 0.0f, 1.0f };
        r.isWireframe = true;

        // UnitSphere는 반지름이 0.5 (지름 1.0)입니다.
        // 우리가 원하는 반지름이 R이라면, 지름은 2R이 되어야 합니다.
        // 따라서 스케일은 (R * 2)가 되어야 합니다.
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
