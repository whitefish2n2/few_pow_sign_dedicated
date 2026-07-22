
#ifndef FPSPROJECTSERVER_GENERATEOBJECTDTO_H
#define FPSPROJECTSERVER_GENERATEOBJECTDTO_H
#pragma once
#include <cstdint>
#include <cstring>

#include "SocketEventType.h"
#include "../../Session/FhishiX/vector/Vector3.h"

struct GenerateObjectDto {
    uint32_t targetId = 0;
    uint8_t prefabId = 0;
    Vector3 position = Vector3::Zero();

    void Clear() { targetId = 0; prefabId = 0; position = Vector3::Zero(); }

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        // 헤더(1) + targetId(4) + prefabId(1) + pos(12)
        return 1 + sizeof(uint32_t) + 1 + sizeof(Vector3);
    }

    void ToBinary(uint8_t* buffer) const {
        size_t offset = 0;
        buffer[offset++] = static_cast<uint8_t>(SocketEventType::GenerateObject);
        std::memcpy(buffer + offset, &targetId, sizeof(uint32_t)); offset += sizeof(uint32_t);
        buffer[offset++] = prefabId;
        std::memcpy(buffer + offset, &position, sizeof(Vector3));
    }
};
#endif //FPSPROJECTSERVER_GENERATEOBJECTDTO_H