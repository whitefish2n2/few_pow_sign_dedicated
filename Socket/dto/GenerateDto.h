#ifndef FPSPROJECTSERVER_GENERATEDTO_H
#define FPSPROJECTSERVER_GENERATEDTO_H
#pragma once
#include <string>
#include <cstdint>
#include <cstring>
#include <vector>

#include "SocketEventType.h"
#include "../../Session/FhishiX/vector/Vector3.h"

struct GenerateEntry {
    uint8_t publicKey = 0;
    uint8_t team      = 0;
    uint8_t characterId    = 0;   // string → 1byte 고정
    Vector3 spawnPos  = Vector3::Zero();
};

struct GenerateDto {
    std::vector<GenerateEntry> players;

    void Clear() { players.clear(); }

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        size_t len = 1;   // 헤더(SocketEventType)
        len += 1;         // playerCount (uint8)
        for (const auto& e : players) {
            len += 1;                        // publicKey
            len += 1;                        // team
            len += 1;  // characterId
            len += sizeof(float) * 3;        // spawnPos
        }
        return len;
    }

    void ToBinary(uint8_t* buffer) const {
        size_t offset = 0;
        buffer[offset++] = static_cast<uint8_t>(SocketEventType::GeneratePlayer);
        buffer[offset++] = static_cast<uint8_t>(players.size());

        for (const auto& e : players) {
            buffer[offset++] = e.publicKey;
            buffer[offset++] = e.team;
            buffer[offset++] = e.characterId;

            std::memcpy(buffer + offset, &e.spawnPos.x, sizeof(float)); offset += sizeof(float);
            std::memcpy(buffer + offset, &e.spawnPos.y, sizeof(float)); offset += sizeof(float);
            std::memcpy(buffer + offset, &e.spawnPos.z, sizeof(float)); offset += sizeof(float);
        }
    }
};
#endif //FPSPROJECTSERVER_GENERATEDTO_H