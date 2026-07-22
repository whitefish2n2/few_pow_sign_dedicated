#ifndef FPSPROJECTSERVER_RELOADNOTIFYDTO_H
#define FPSPROJECTSERVER_RELOADNOTIFYDTO_H
#pragma once
#include <cstdint>
#include <cstring>

#include "SocketEventType.h"

// S2P 리로드통보: 누가(playerKey) 어느 슬롯(slot)을 리로드해 잔탄(currentAmmo)이 얼마인지
struct ReloadNotifyDto {
    uint8_t  playerKey   = 0;
    uint8_t  slot        = 0;
    uint16_t currentAmmo = 0;

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        return 1 + 1 + 1 + sizeof(uint16_t);
    }

    void ToBinary(uint8_t* buffer) const {
        size_t off = 0;
        buffer[off++] = static_cast<uint8_t>(SocketEventType::ReloadNotify);
        buffer[off++] = playerKey;
        buffer[off++] = slot;
        std::memcpy(buffer + off, &currentAmmo, sizeof(uint16_t));
    }
};
#endif //FPSPROJECTSERVER_RELOADNOTIFYDTO_H
