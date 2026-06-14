//
// Created by white on 26. 6. 10..
//

#pragma once
#include <cstdint>
#include "SocketEventType.h"

struct ProgressNotifyDto {
    uint8_t publicId; // 누구의 로딩 진행도인가?
    uint8_t progress; // 몇 퍼센트인가? (0~100)

    ProgressNotifyDto(uint8_t id, uint8_t prog) : publicId(id), progress(prog) {}
    ProgressNotifyDto() = default;

    size_t GetDtoBinaryLength() const {
        return 1 + sizeof(publicId) + sizeof(progress); // 헤더(1) + ID(1) + 퍼센트(1) = 3 bytes
    }

    void ToBinary(uint8_t* buffer) const {
        size_t offset = 0;
        buffer[offset++] = static_cast<uint8_t>(SocketEventType::ProgressNotify); // 새로 추가할 이벤트 타입

        std::memcpy(buffer + offset, &publicId, sizeof(publicId));
        offset += sizeof(publicId);

        std::memcpy(buffer + offset, &progress, sizeof(progress));
    }
};
struct LoadingProgressDto {
    uint8_t progress;

    void Parse(const uint8_t* payload, size_t length) {
        if (length >= 1) {
            progress = payload[0];
        }
    }
};