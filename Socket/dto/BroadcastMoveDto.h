#ifndef BROADCASTMOVEDTO_H
#define BROADCASTMOVEDTO_H
#pragma once
#include <cstdint>
#include <cstring>
#include <vector>

#include "SocketEventType.h"
#include "../../Session/FhishiX/vector/Vector3.h"

struct ObjectMoveEntry {
    uint32_t targetId = 0;
    Vector3 position = Vector3::Zero();
    Vector3 rotation = Vector3::Zero();   // euler
};

struct BroadcastMoveDto {
    std::vector<ObjectMoveEntry> objects;

    void Clear() { objects.clear(); }

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        // 헤더(1) + 개수(1) + N*(targetId 4 + pos 12 + rot 12)
        return 1 + 1 + objects.size() * (sizeof(uint32_t) + sizeof(Vector3) * 2);
    }

    void ToBinary(uint8_t* buffer) const {
        size_t offset = 0;
        buffer[offset++] = static_cast<uint8_t>(SocketEventType::ObjectMove);
        buffer[offset++] = static_cast<uint8_t>(objects.size());

        for (const auto& e : objects) {
            std::memcpy(buffer + offset, &e.targetId, sizeof(uint32_t)); offset += sizeof(uint32_t);
            std::memcpy(buffer + offset, &e.position, sizeof(Vector3)); offset += sizeof(Vector3);
            std::memcpy(buffer + offset, &e.rotation, sizeof(Vector3)); offset += sizeof(Vector3);
        }
    }
};
#endif //BROADCASTMOVEDTO_H