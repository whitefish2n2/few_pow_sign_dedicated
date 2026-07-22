#ifndef FPSPROJECTSERVER_GETWEAPONNOTIFYDTO_H
#define FPSPROJECTSERVER_GETWEAPONNOTIFYDTO_H
#pragma once
#include <cstdint>
#include <cstring>

#include "SocketEventType.h"

// S2P 픽업통보: 누가(pickerKey) 어떤 무기(weaponTargetId)를 어느 슬롯(slot)에 주웠는지 + 현재 장착슬롯
struct GetWeaponNotifyDto {
    uint8_t  pickerKey      = 0;
    uint32_t weaponTargetId = 0;
    uint8_t  slot           = 0;
    uint8_t  holdingSlot    = 0xFF;   // 0xFF = 빈손

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        return 1 + 1 + sizeof(uint32_t) + 1 + 1;
    }

    void ToBinary(uint8_t* buffer) const {
        size_t off = 0;
        buffer[off++] = static_cast<uint8_t>(SocketEventType::GetWeaponNotify);
        buffer[off++] = pickerKey;
        std::memcpy(buffer + off, &weaponTargetId, sizeof(uint32_t)); off += sizeof(uint32_t);
        buffer[off++] = slot;
        buffer[off++] = holdingSlot;
    }
};
#endif //FPSPROJECTSERVER_GETWEAPONNOTIFYDTO_H
