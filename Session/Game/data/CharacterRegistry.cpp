//
// Created by white on 26. 6. 30..
//

#include "./CharacterRegistry.h"
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

struct CharacterInfo;
std::vector<CharacterInfo> CharacterRegistry::_data;

void CharacterRegistry::Init(const std::string& jsonPath) {
    std::ifstream f(jsonPath);
    if (!f) throw std::runtime_error("CharacterData.json not found: " + jsonPath);
    auto j = nlohmann::json::parse(f);
    for (const auto& c : j["characterList"]) {
        CharacterInfo info;
        info.id       = c["id"].get<uint8_t>();
        info.prefabId = c["prefabId"].get<uint32_t>();
        info.maxHp    = c["baseStats"]["maxHp"].get<int>();
        info.speed    = c["baseStats"]["speed"].get<float>();
        if (info.id >= _data.size()) _data.resize(info.id + 1);
        _data[info.id] = info;
    }
}

const CharacterInfo* CharacterRegistry::Get(uint8_t id) {
    if (id >= _data.size()) return nullptr;
    return &_data[id];
}