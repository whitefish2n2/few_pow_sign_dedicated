//
// Created by white on 26. 5. 28..
//

#ifndef FPSPROJECTSERVER_WEAPON_H
#define FPSPROJECTSERVER_WEAPON_H
#include <string>

#include "../../FhishiX/vector/Vector3.h"
#include "../Definition/Component.h"

struct WeaponInfo;

class Weapon final : public Component<Weapon> {
public:
    static constexpr bool DO_UPDATE = false;   // 발사 때만 동작, 매틱 업데이트 불요

    uint8_t weaponId = 0;      // 데이터 id (= weaponName, WeaponRegistry 키)
    int currentAmmo = 0;
    std::chrono::steady_clock::time_point lastShotTime;

    void Start() override;
    void ParseFromString(const std::string& arg) override;

    bool TryShoot();

    void DropToWorld(const Vector3& pos, const Vector3& impulse, const Vector3& viewRot,
                     uint8_t dropperKey, uint8_t holdingSlotAfter) const;
    const WeaponInfo* GetInfo() const;         // WeaponRegistry 룩업
};
#endif //FPSPROJECTSERVER_WEAPON_H