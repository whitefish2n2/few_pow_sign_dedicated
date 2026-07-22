#ifndef FPSPROJECTSERVER_WEAPONINVENTORY_H
#define FPSPROJECTSERVER_WEAPONINVENTORY_H
#include "../Definition/Component.h"
#include "../Definition/ComponentHandle.h"
#include "Weapon.h"

class WeaponInventory final : public Component<WeaponInventory> {
public:
    static constexpr bool DO_UPDATE = false;
    static constexpr int SLOT_COUNT = 5;          // WeaponType::COUNT

    ComponentHandle<Weapon> slots[SLOT_COUNT];    // 기본생성 = isNull() = 빈 슬롯
    int holdingSlot = -1;                         // -1 = 빈손

    // 조회
    ComponentHandle<Weapon> GetHolding() const;
    bool IsSlotEmpty(int slot) const;

    // 인벤토리 조작(서버 권위 · 클라 WeaponSystem 포팅)
    ComponentHandle<Weapon> Pickup(ComponentHandle<Weapon> weapon); // 반환=밀려난 기존무기(월드로 드롭)
    bool SwapDir(bool up);                                          // 위/아래 존재슬롯으로(없으면 캔슬)
    bool SwapTo(int slot);                                  //해당 슬롯에 무기 있음 장착
    ComponentHandle<Weapon> DropHolding();                          // 반환=드롭된 무기(월드로 재활성)
    void DropAll(uint8_t dropperKey);                               // 슬롯 전부 바닥에 드롭(사망 등), 자기 위치 기준
    bool Reload();
    bool ConsumeAmmo();                                             // 발사 시 1 감소(-1=무한)
};
#endif //FPSPROJECTSERVER_WEAPONINVENTORY_H
