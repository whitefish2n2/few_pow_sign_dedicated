//
// Created by white on 26. 6. 29..
//

#ifndef FPSPROJECTSERVER_BROADCASTPLAYERMOVEDTO_H
#define FPSPROJECTSERVER_BROADCASTPLAYERMOVEDTO_H
#pragma once
#include <cstdint>
#include <cstring>
#include <vector>

#include "SocketEventType.h"
#include "../../Session/FhishiX/vector/Vector3.h"

struct PlayerMoveEntry {
    uint8_t publicKey = 0;
    Vector3 position = Vector3::Zero();
    Vector3 rotation = Vector3::Zero();   // x=pitch, y=yaw
    Vector3 velocity = Vector3::Zero();   // 클라 보간/애니용
};

struct BroadcastPlayerMoveDto {
    std::vector<PlayerMoveEntry> players;

    void Clear() { players.clear(); }

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        // 헤더(1) + 인원수(1) + N*(publicKey 1 + pos 12 + rot 12 + vel 12)
        return 1 + 1 + players.size() * (1 + sizeof(Vector3) * 3);
    }

    void ToBinary(uint8_t* buffer) const {
        size_t offset = 0;
        buffer[offset++] = static_cast<uint8_t>(SocketEventType::PlayerMove);
        buffer[offset++] = static_cast<uint8_t>(players.size());

        for (const auto& e : players) {
            buffer[offset++] = e.publicKey;
            std::memcpy(buffer + offset, &e.position, sizeof(Vector3)); offset += sizeof(Vector3);
            std::memcpy(buffer + offset, &e.rotation, sizeof(Vector3)); offset += sizeof(Vector3);
            std::memcpy(buffer + offset, &e.velocity, sizeof(Vector3)); offset += sizeof(Vector3);
        }
    }
};
#endif //FPSPROJECTSERVER_BROADCASTPLAYERMOVEDTO_H