#ifndef FPSPROJECTSERVER_DEATHDTO_H
#define FPSPROJECTSERVER_DEATHDTO_H
#pragma once
#include <cstdint>

#include "SocketEventType.h"

struct DeathDto {
    uint8_t victimKey = 0;
    uint8_t killerKey = 0;

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        return 1 + 1 + 1;   // 헤더 + victimKey + killerKey
    }

    void ToBinary(uint8_t* buffer) const {
        buffer[0] = static_cast<uint8_t>(SocketEventType::Death);
        buffer[1] = victimKey;
        buffer[2] = killerKey;
    }
};
#endif //FPSPROJECTSERVER_DEATHDTO_H
