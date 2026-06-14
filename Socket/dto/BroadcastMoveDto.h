//
// Created by white on 25. 5. 23.
//

#ifndef BROADCASTMOVEDTO_H
#define BROADCASTMOVEDTO_H
#include <cstdint>

#include "SocketEventType.h"

#include "../../Session/FhishiX/vector/Vector3.h"
class BroadcastMoveDto {
    public:
    uint32_t targetId;        // 4 bytes: 움직인 주체 (SecretKey 아님!)
    Vector3 currentPosition;  // 12 bytes: 서버에서 확정된 절대 좌표
    Vector3 rotation;         // 12 bytes: 바라보는 방향
    Vector3 velocity;      // 12 bytes: 속도: 클라 보간용(애니메이션 재생용)

    // 생성자 (서버 로직에서 묶어서 던지기 편하게)
    BroadcastMoveDto(uint32_t id, const Vector3& pos, const Vector3& rot, const Vector3& velocity)
        : targetId(id), currentPosition(pos), rotation(rot), velocity(velocity) {}

    // 프레임워크 요구 함수 1: 길이
    size_t GetDtoBinaryLength() const {
        // 헤더(1) + ID(4) + Pos(12) + Rot(12) + Input(8) = 37 bytes
        return 1 + sizeof(targetId) + sizeof(currentPosition) + sizeof(rotation) + sizeof(velocity);
    }

    // 프레임워크 요구 함수 2: 직렬화 (안전한 memcpy 방식)
    void ToBinary(uint8_t* buffer) const {
        size_t offset = 0;

        // 1. 헤더 (1 byte)
        buffer[offset] = static_cast<uint8_t>(SocketEventType::Move);
        offset += 1;

        // 2. Target ID (4 bytes)
        std::memcpy(buffer + offset, &targetId, sizeof(targetId));
        offset += sizeof(targetId);

        // 3. Position (12 bytes)
        std::memcpy(buffer + offset, &currentPosition, sizeof(currentPosition));
        offset += sizeof(currentPosition);

        // 4. Rotation (12 bytes)
        std::memcpy(buffer + offset, &rotation, sizeof(rotation));
        offset += sizeof(rotation);

        // 5. Velocity (12 bytes) - 보간용
        std::memcpy(buffer + offset, &velocity, sizeof(velocity));
    }
};



#endif //BROADCASTMOVEDTO_H
