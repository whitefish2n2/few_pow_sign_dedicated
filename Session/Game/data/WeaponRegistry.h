#ifndef FPSPROJECTSERVER_WEAPONREGISTRY_H
#define FPSPROJECTSERVER_WEAPONREGISTRY_H
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "../../FhishiX/vector/Vector3.h"


struct WeaponInfo {
    uint8_t     id = 0;
    std::string weaponName;     // 라벨/크로스레퍼런스
    std::string type;           // WeaponType(슬롯) — 19b
    float headDamage = 0;
    float bodyDamage = 0;
    float lagDamage  = 0;
    int   maxAmmo    = 0;
    float termToShot = 0;
    Vector3 handlePosition = Vector3::Zero();
    Vector3 handleObjectRotation = Vector3::Zero();
};

class WeaponRegistry {
public:
    static void Init(const std::string& jsonPath);
    static const WeaponInfo* Get(uint8_t id);   // 없으면 nullptr
private:
    static std::vector<WeaponInfo> _data;       // id 인덱스, read-only
};
#endif //FPSPROJECTSERVER_WEAPONREGISTRY_H