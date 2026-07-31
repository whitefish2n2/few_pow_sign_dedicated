
#include <random>

#include "WeaponInventory.h"
#include "../Definition/ComponentFactory.h"
#include "../../Game/data/WeaponRegistry.h"
#include "../../Game/data/WeaponType.h"
#include "PlayerComponent.h"
namespace {
    int SlotOfWeapon(ComponentHandle<Weapon> h) {
        Weapon* w = h.operator->();
        if (!w) return -1;
        const WeaponInfo* info = w->GetInfo();
        return info ? WeaponTypeToSlot(info->type) : -1;
    }
}

ComponentHandle<Weapon> WeaponInventory::GetHolding() const {
    if (holdingSlot < 0 || holdingSlot >= SLOT_COUNT) return ComponentHandle<Weapon>::NULLPTR();
    return slots[holdingSlot];
}

bool WeaponInventory::IsSlotEmpty(int slot) const {
    if (slot < 0 || slot >= SLOT_COUNT) return true;
    return slots[slot].isNull();
}

ComponentHandle<Weapon> WeaponInventory::Pickup(ComponentHandle<Weapon> weapon) {
    int slot = SlotOfWeapon(weapon);
    if (slot < 0) return ComponentHandle<Weapon>::NULLPTR();

    ComponentHandle<Weapon> before = slots[slot];   // 밀려나는 기존무기(있으면)
    slots[slot] = weapon;

    if (holdingSlot < 0 || holdingSlot == slot) holdingSlot = slot;   // 빈손/그슬롯 보유중이면 장착
    return before;
}

bool WeaponInventory::SwapDir(bool up) {
    if (up) {
        for (int i = holdingSlot + 1; i < SLOT_COUNT; ++i)
            if (!slots[i].isNull()) { holdingSlot = i; return true; }
    } else {
        for (int i = holdingSlot - 1; i >= 0; --i)
            if (!slots[i].isNull()) { holdingSlot = i; return true; }
    }
    return false;   // 그 방향에 무기 없음 → 캔슬
}

bool WeaponInventory::SwapTo(int slot) {
    if (slot < 0 || slot >= SLOT_COUNT || slots[slot].isNull()) return false;
    holdingSlot = slot;
    return true;
}

ComponentHandle<Weapon> WeaponInventory::DropHolding() {
    if (holdingSlot < 0 || slots[holdingSlot].isNull()) return ComponentHandle<Weapon>::NULLPTR();

    ComponentHandle<Weapon> dropped = slots[holdingSlot];
    int prev = holdingSlot;
    slots[prev] = ComponentHandle<Weapon>::NULLPTR();
    holdingSlot = -1;

    // 다음 가용 무기 자동 장착(위 우선 → 아래)
    for (int i = prev + 1; i < SLOT_COUNT; ++i) if (!slots[i].isNull()) { holdingSlot = i; break; }
    if (holdingSlot < 0)
        for (int i = prev - 1; i >= 0; --i) if (!slots[i].isNull()) { holdingSlot = i; break; }

    return dropped;
}

bool WeaponInventory::Reload() {
    Weapon* w = GetHolding().operator->();
    if (!w) return false;
    const WeaponInfo* info = w->GetInfo();
    if (!info || info->maxAmmo < 0) return false;        // 무한탄약은 리로드 무의미
    if (w->currentAmmo >= info->maxAmmo) return false;   // 이미 풀
    w->currentAmmo = info->maxAmmo;
    return true;
}

void WeaponInventory::DropAll(uint8_t dropperKey) {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    auto pc = gameObject->GetComponent<PlayerComponent>();
    Vector3 aimOrigin = pc.isNull() ? Vector3(0.0f, 1.74f, 0.0f) : pc->aimOrigin;
    Vector3 pos = gameObject->transform.GetPosition() + aimOrigin;
    for (int i = 0; i < SLOT_COUNT; ++i) {
        Weapon* w = slots[i].operator->();
        if (w != nullptr) {
            Vector3 impulse(dist(rng), 0.0f, dist(rng));   // 주변으로 살짝 흩뿌리는 정도(대략 1 안팎)
            w->DropToWorld(pos, impulse, Vector3::Zero(), dropperKey, 0xFF);
        }
        slots[i] = ComponentHandle<Weapon>::NULLPTR();
    }
    holdingSlot = -1;
}

bool WeaponInventory::ConsumeAmmo() {
    Weapon* w = GetHolding().operator->();
    if (!w) return false;
    const WeaponInfo* info = w->GetInfo();
    if (info && info->maxAmmo < 0) return true;   // 무한탄약(-1)
    if (w->currentAmmo <= 0) return false;
    w->currentAmmo--;
    return true;
}


REGISTER_COMPONENT(WeaponInventory)