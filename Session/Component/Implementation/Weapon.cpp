#include "Weapon.h"
#include <sstream>

#include "Interactable.h"
#include "WeaponInventory.h"
#include "../../../ObjectPool.h"
#include "../../../util/StringUtil.h"
#include "../Definition/ComponentFactory.h"
#include "../../Game/data/WeaponRegistry.h"
#include "../../Game/data/WeaponType.h"
#include "../../../Socket/dto/GetWeaponNotifyDto.h"
#include "../../../Socket/dto/DropWeaponNotifyDto.h"

void Weapon::Start() {
    Component<Weapon>::Start();
    if (const WeaponInfo* info = WeaponRegistry::Get(weaponId)) currentAmmo = info->maxAmmo;

    auto interactable = gameObject->GetComponent<Interactable>();
    if (interactable.isNull()) return;
    ComponentHandle<Weapon> self = MakeHandle();
    interactable->onInteract.AddListener([self](Player* player) {
        Weapon* w = self.operator->();
        if (w == nullptr || player == nullptr || player->playerComponent.isNull()) return;
        auto inventory = player->playerComponent->gameObject->GetComponent<WeaponInventory>();
        if (inventory.isNull()) return;

        const WeaponInfo* info = w->GetInfo();
        int slot = (info != nullptr) ? WeaponTypeToSlot(info->type) : -1;
        if (slot < 0) return;

        ComponentHandle<Weapon> pushedOut = inventory->Pickup(self);

        // 클라 Get() 미러: 재상호작용 차단 + 월드에서 숨김
        auto selfInteract = w->gameObject->GetComponent<Interactable>();
        if (!selfInteract.isNull()) selfInteract->isInteractable = false;
        w->gameObject->SetActive(false);

        // 밀려난 기존무기 → 손 위치에서 드롭
        Weapon* droppedW = pushedOut.operator->();
        if (droppedW != nullptr) {
            PlayerMoveSnapshot snap = player->playerComponent->GetMoveSnapshot();
            const WeaponInfo* dInfo = droppedW->GetInfo();   // ※버려지는 무기의 handlePosition (줍는 무기 X)
            Vector3 hp = (dInfo != nullptr) ? dInfo->handlePosition : Vector3::Zero();
            float pitchRad = snap.rotation.x * (3.14159265f / 180.0f);
            float yawRad   = snap.rotation.y * (3.14159265f / 180.0f);
            float cp = std::cos(pitchRad), sp = std::sin(pitchRad);
            float cy = std::cos(yawRad),   sy = std::sin(yawRad);
            Vector3 pitched(hp.x, hp.y * cp - hp.z * sp, hp.y * sp + hp.z * cp);   // Rx(pitch)
            Vector3 off(pitched.x * cy + pitched.z * sy,                            // Ry(yaw)
                        pitched.y,
                        -pitched.x * sy + pitched.z * cy);
            droppedW->DropToWorld(snap.position + player->playerComponent->aimOrigin + off,
                                  Vector3(sy, 0.0f, cy),
                                  snap.rotation,
                                  player->publicKey,
                                  static_cast<uint8_t>(inventory->holdingSlot < 0 ? 0xFF : inventory->holdingSlot));
        }

        // GetWeaponNotify 방송
        GetWeaponNotifyDto* raw = ObjectPool<GetWeaponNotifyDto>::GetInstance().Acquire();
        raw->pickerKey      = player->publicKey;
        raw->weaponTargetId = static_cast<uint32_t>(w->gameObject.GetId());
        raw->slot           = static_cast<uint8_t>(slot);
        raw->holdingSlot    = static_cast<uint8_t>(inventory->holdingSlot < 0 ? 0xFF : inventory->holdingSlot);
        auto dto = std::unique_ptr<GetWeaponNotifyDto, void(*)(GetWeaponNotifyDto*)>(
            raw, [](GetWeaponNotifyDto* p) { ObjectPool<GetWeaponNotifyDto>::GetInstance().Release(p); });

        BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
        rawEvent->type = SocketEventType::GetWeaponNotify;
        rawEvent->payload = std::move(dto);
        rawEvent->target.clear();
        std::shared_ptr<BroadCastEvent> event(rawEvent, [](BroadCastEvent* p) {
            p->payload = nullptr;
            ObjectPool<BroadCastEvent>::GetInstance().Release(p);
        });
        w->gameSession->BroadcastEvent(event);
    });
}

void Weapon::DropToWorld(const Vector3& pos, const Vector3& impulse, const Vector3& viewRot,
                         uint8_t dropperKey, uint8_t holdingSlotAfter) const {
    gameObject->transform.SetPosition(pos);
    const WeaponInfo* info = GetInfo();   // 카메라(pitch,yaw) × 핸들회전 = 들고 있던 월드 포즈
    gameObject->transform.SetRotation(
        Quaternion::FromEuler(Vector3(viewRot.x, viewRot.y, 0.0f))
      * Quaternion::FromEuler(info != nullptr ? info->handleObjectRotation : Vector3::Zero()));
    gameObject->SetActive(true);
    Rigidbody* rbPtr = gameObject->GetComponent<Rigidbody>().operator->();
    if (rbPtr != nullptr) rbPtr->AddImpulse(impulse);   // 클라 rb.AddForce(force, ForceMode.Impulse) 미러
    auto interactable = gameObject->GetComponent<Interactable>();
    if (!interactable.isNull()) interactable->isInteractable = true;

    // DropWeaponNotify 방송
    DropWeaponNotifyDto* raw = ObjectPool<DropWeaponNotifyDto>::GetInstance().Acquire();
    raw->dropperKey     = dropperKey;
    raw->weaponTargetId = static_cast<uint32_t>(gameObject.GetId());
    raw->position       = pos;
    raw->holdingSlot    = holdingSlotAfter;
    auto dto = std::unique_ptr<DropWeaponNotifyDto, void(*)(DropWeaponNotifyDto*)>(
        raw, [](DropWeaponNotifyDto* p) { ObjectPool<DropWeaponNotifyDto>::GetInstance().Release(p); });

    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
    rawEvent->type = SocketEventType::DropWeaponNotify;
    rawEvent->payload = std::move(dto);
    rawEvent->target.clear();
    std::shared_ptr<BroadCastEvent> event(rawEvent, [](BroadCastEvent* p) {
        p->payload = nullptr;
        ObjectPool<BroadCastEvent>::GetInstance().Release(p);
    });
    gameSession->BroadcastEvent(event);
}
void Weapon::ParseFromString(const std::string& arg) {
    std::stringstream ss(arg);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        size_t delimPos = line.find(':');
        if (delimPos == std::string::npos) continue;
        std::string key = StringUtils::Trim(line.substr(0, delimPos));
        std::string val = StringUtils::Trim(line.substr(delimPos + 1));
        if (key == "WeaponId") this->weaponId = static_cast<uint8_t>(std::stoi(val));
    }
}

bool Weapon::TryShoot() {
    const WeaponInfo* info = GetInfo();
    if (info == nullptr) return false;

    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - lastShotTime).count();
    if (elapsed < info->termToShot) return false;   // 연사율 게이트

    if (info->maxAmmo >= 0) {                       // -1=무한탄약
        if (currentAmmo <= 0) return false;
        currentAmmo--;
    }
    lastShotTime = now;
    return true;
}

const WeaponInfo* Weapon::GetInfo() const { return WeaponRegistry::Get(weaponId); }

REGISTER_COMPONENT(Weapon)