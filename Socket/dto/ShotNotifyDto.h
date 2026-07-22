#ifndef FPSPROJECTSERVER_SHOTNOTIFYDTO_H
#define FPSPROJECTSERVER_SHOTNOTIFYDTO_H
#pragma once
#include <cstdint>
#include <cstring>

#include "SocketEventType.h"
#include "../../Session/FhishiX/vector/Vector3.h"

// S2P 발사통보: 누가(playerKey) 어디서(origin) 어느 방향(dir)으로 쐈는지 — 사운드 텔레메트리용
struct ShotNotifyDto {
    uint8_t playerKey = 0;
    Vector3 origin     = Vector3::Zero();
    Vector3 dir        = Vector3::Zero();

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        return 1 + 1 + sizeof(Vector3) * 2;
    }

    void ToBinary(uint8_t* buffer) const {
        size_t off = 0;
        buffer[off++] = static_cast<uint8_t>(SocketEventType::ShotNotify);
        buffer[off++] = playerKey;
        std::memcpy(buffer + off, &origin, sizeof(Vector3)); off += sizeof(Vector3);
        std::memcpy(buffer + off, &dir, sizeof(Vector3));
    }
};
#endif //FPSPROJECTSERVER_SHOTNOTIFYDTO_H
