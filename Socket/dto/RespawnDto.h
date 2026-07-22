//
// Created by white on 26. 6. 30..
//

#ifndef FPSPROJECTSERVER_RESPAWNDTO_H
#define FPSPROJECTSERVER_RESPAWNDTO_H
#pragma once
#include <cstdint>
#include <cstring>
#include <vector>

#include "SocketEventType.h"
#include "../../Session/FhishiX/vector/Vector3.h"

struct RespawnEntry {
    uint8_t publicKey = 0;
    Vector3 position = Vector3::Zero();
};

struct RespawnDto {
    std::vector<RespawnEntry> players;   // count 기반: 개별 사망 리스폰 흡수

    void Clear() { players.clear(); }

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        // 헤더(1) + 인원수(1) + N*(publicKey 1 + pos 12)
        return 1 + 1 + players.size() * (1 + sizeof(Vector3));
    }

    void ToBinary(uint8_t* buffer) const {
        size_t offset = 0;
        buffer[offset++] = static_cast<uint8_t>(SocketEventType::RespawnPlayer);
        buffer[offset++] = static_cast<uint8_t>(players.size());
        for (const auto& e : players) {
            buffer[offset++] = e.publicKey;
            std::memcpy(buffer + offset, &e.position, sizeof(Vector3)); offset += sizeof(Vector3);
        }
    }
};
#endif //FPSPROJECTSERVER_RESPAWNDTO_H