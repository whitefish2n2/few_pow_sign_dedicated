#ifndef FPSPROJECTSERVER_PHASECHANGENOTIFYDTO_H
#define FPSPROJECTSERVER_PHASECHANGENOTIFYDTO_H
#pragma once
#include <cstdint>
#include <cstring>

#include "SocketEventType.h"

// S2P 페이즈 전환 통보: 어떤 페이즈로 들어갔는지 + 그 페이즈 지속시간(클라 카운트다운용)
struct PhaseChangeNotifyDto {
    uint8_t phase    = 0;
    float   duration = 0.0f;

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        return 1 + 1 + sizeof(float);
    }

    void ToBinary(uint8_t* buffer) const {
        size_t off = 0;
        buffer[off++] = static_cast<uint8_t>(SocketEventType::PhaseChangeNotify);
        buffer[off++] = phase;
        std::memcpy(buffer + off, &duration, sizeof(float));
    }
};
#endif //FPSPROJECTSERVER_PHASECHANGENOTIFYDTO_H
