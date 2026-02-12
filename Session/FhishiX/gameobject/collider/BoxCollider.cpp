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

    CalculateAABB();
}

Renderer BoxCollider::GetRenderer() {
}
