//
// Created by white on 26. 5. 28..
//

#include "CharacterSummonPosition.h"

#include "../Definition/ComponentFactory.h"

void CharacterSummonPosition::ParseFromString(const std::string &arg) {

    std::stringstream ss(arg);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        size_t delimPos = line.find(": ");
        if (delimPos == std::string::npos) continue;

        std::string key = line.substr(0, delimPos);
        std::string val = line.substr(delimPos + 2);

        if (key == "TeamId") {
            this->teamId = std::stoi(val);
        }
    }
}

REGISTER_COMPONENT(CharacterSummonPosition);
