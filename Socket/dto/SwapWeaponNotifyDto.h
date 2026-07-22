#ifndef FPSPROJECTSERVER_SWAPWEAPONNOTIFYDTO_H
#define FPSPROJECTSERVER_SWAPWEAPONNOTIFYDTO_H
#pragma once
#include <cstdint>
#include <cstddef>

#include "SocketEventType.h"

// S2P 스왑통보: 누가(playerKey) 어느 슬롯(holdingSlot)을 장착 중인지
struct SwapWeaponNotifyDto {
    uint8_t playerKey   = 0;
    uint8_t holdingSlot = 0;

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        return 1 + 1 + 1;
    }

    void ToBinary(uint8_t* buffer) const {
        buffer[0] = static_cast<uint8_t>(SocketEventType::SwapWeaponNotify);
        buffer[1] = playerKey;
        buffer[2] = holdingSlot;
    }
};
#endif //FPSPROJECTSERVER_SWAPWEAPONNOTIFYDTO_H
