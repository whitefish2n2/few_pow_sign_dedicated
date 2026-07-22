#ifndef FPSPROJECTSERVER_DROPWEAPONNOTIFYDTO_H
#define FPSPROJECTSERVER_DROPWEAPONNOTIFYDTO_H
#pragma once
#include <cstdint>
#include <cstring>

#include "SocketEventType.h"
#include "../../Session/FhishiX/vector/Vector3.h"

// S2P 드롭통보: 누가(dropperKey) 어떤 무기(weaponTargetId)를 어디(position)에 버렸는지 + 드롭 후 장착슬롯
struct DropWeaponNotifyDto {
    uint8_t  dropperKey     = 0;
    uint32_t weaponTargetId = 0;
    Vector3  position       = Vector3::Zero();
    uint8_t  holdingSlot    = 0xFF;   // 0xFF = 빈손

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        return 1 + 1 + sizeof(uint32_t) + sizeof(Vector3) + 1;
    }

    void ToBinary(uint8_t* buffer) const {
        size_t off = 0;
        buffer[off++] = static_cast<uint8_t>(SocketEventType::DropWeaponNotify);
        buffer[off++] = dropperKey;
        std::memcpy(buffer + off, &weaponTargetId, sizeof(uint32_t)); off += sizeof(uint32_t);
        std::memcpy(buffer + off, &position, sizeof(Vector3)); off += sizeof(Vector3);
        buffer[off++] = holdingSlot;
    }
};
#endif //FPSPROJECTSERVER_DROPWEAPONNOTIFYDTO_H
