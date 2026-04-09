//
// Created by white on 25. 12. 26..
//

#include "BoxCollider.h"
#include "../GameObjectArgument.h"
#include "../../../Component/Definition/ComponentFactory.h"



REGISTER_COMPONENT(BoxCollider);

void BoxCollider::ParseFromString(const std::string& arg) {
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
        else if (key == "Size") {
            this->size =Vector3::ParseVector3(val);
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
    std::cout << "owner setting: " << this->gameObject->name << std::endl;

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
