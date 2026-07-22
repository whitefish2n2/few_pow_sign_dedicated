#include "./WeaponRegistry.h"
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

#include "../../FhishiX/vector/Vector3.h"

std::vector<WeaponInfo> WeaponRegistry::_data;

void WeaponRegistry::Init(const std::string& jsonPath) {
    std::ifstream f(jsonPath);
    if (!f) throw std::runtime_error("WeaponData.json not found: " + jsonPath);
    auto j = nlohmann::json::parse(f);
    for (const auto& w : j["weaponList"]) {
        WeaponInfo info;
        info.id         = w["id"].get<uint8_t>();
        info.weaponName = w["weaponName"].get<std::string>();
        info.type       = w["type"].get<std::string>();
        info.headDamage = w["headDamage"].get<float>();
        info.bodyDamage = w["bodyDamage"].get<float>();
        info.lagDamage  = w.value("lagDamage", 0.0f);
        info.maxAmmo    = w["maxAmmo"].get<int>();
        info.termToShot = w["termToShot"].get<float>();
        info.handlePosition = Vector3(w["handlePosition"]["x"].get<float>(),
                                      w["handlePosition"]["y"].get<float>(),
                                      w["handlePosition"]["z"].get<float>());
        info.handleObjectRotation = Vector3(w["handleObjectRotation"]["x"].get<float>(),
                                    w["handleObjectRotation"]["y"].get<float>(),
                                    w["handleObjectRotation"]["z"].get<float>());
        if (info.id >= _data.size()) _data.resize(info.id + 1);
        _data[info.id] = info;
    }
}

const WeaponInfo* WeaponRegistry::Get(uint8_t id) {
    if (id >= _data.size()) return nullptr;
    return &_data[id];
}