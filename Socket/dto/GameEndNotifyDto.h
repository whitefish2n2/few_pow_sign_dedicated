#ifndef FPSPROJECTSERVER_GAMEENDNOTIFYDTO_H
#define FPSPROJECTSERVER_GAMEENDNOTIFYDTO_H
#pragma once
#include <cstdint>
#include <cstring>

#include "SocketEventType.h"

// S2P 게임 종료 통보: 승자 팀
struct GameEndNotifyDto {
    uint8_t winningTeam = 0;

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        return 1 + 1;
    }

    void ToBinary(uint8_t* buffer) const {
        size_t off = 0;
        buffer[off++] = static_cast<uint8_t>(SocketEventType::GameEndNotify);
        buffer[off++] = winningTeam;
    }
};
#endif //FPSPROJECTSERVER_GAMEENDNOTIFYDTO_H
