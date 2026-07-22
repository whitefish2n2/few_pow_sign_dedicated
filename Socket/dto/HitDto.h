#ifndef FPSPROJECTSERVER_HITDTO_H
#define FPSPROJECTSERVER_HITDTO_H
#pragma once
#include <cstdint>
#include <cstring>

#include "SocketEventType.h"
#include "../../Session/FhishiX/vector/Vector3.h"

struct HitDto {
    uint8_t  victimKey    = 0;
    uint8_t  attackerKey  = 0;
    uint8_t  hitPart      = 0;   // 0=body, 1=head
    uint16_t remainingHp  = 0;   // 서버 권위 잔여 HP (clamp >= 0)
    Vector3  hitPosition  = Vector3::Zero();   // 이펙트 표시용 (레이캐스트 맞은 지점)

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        return 1 + 1 + 1 + 1 + sizeof(uint16_t) + sizeof(Vector3);
    }

    void ToBinary(uint8_t* buffer) const {
        size_t offset = 0;
        buffer[offset++] = static_cast<uint8_t>(SocketEventType::HitNotify);
        buffer[offset++] = victimKey;
        buffer[offset++] = attackerKey;
        buffer[offset++] = hitPart;
        std::memcpy(buffer + offset, &remainingHp, sizeof(uint16_t)); offset += sizeof(uint16_t);
        std::memcpy(buffer + offset, &hitPosition, sizeof(Vector3));
    }
};
#endif //FPSPROJECTSERVER_HITDTO_H
