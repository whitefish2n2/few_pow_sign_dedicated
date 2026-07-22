#ifndef FPSPROJECTSERVER_ROUNDENDNOTIFYDTO_H
#define FPSPROJECTSERVER_ROUNDENDNOTIFYDTO_H
#pragma once
#include <cstdint>
#include <cstring>

#include "SocketEventType.h"

// S2P 라운드 종료 통보: 이번 라운드 승리팀 + 그 팀의 누적 스코어(클라는 이걸 누적해서 스코어보드 구성)
struct RoundEndNotifyDto {
    uint8_t winningTeam      = 0;
    uint8_t winningTeamScore = 0;

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        return 1 + 1 + 1;
    }

    void ToBinary(uint8_t* buffer) const {
        size_t off = 0;
        buffer[off++] = static_cast<uint8_t>(SocketEventType::RoundEndNotify);
        buffer[off++] = winningTeam;
        buffer[off++] = winningTeamScore;
    }
};
#endif //FPSPROJECTSERVER_ROUNDENDNOTIFYDTO_H
