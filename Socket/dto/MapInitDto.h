//
// Created by white on 26. 6. 2..
//

#ifndef FPSPROJECTSERVER_MAPINITDTO_H
#define FPSPROJECTSERVER_MAPINITDTO_H


#pragma once
#include <string>
#include <cstdint>
#include <cstring>
#include <ranges>

#include "SocketEventType.h"
#include "../../Session/FhishiX/vector/Vector3.h"
#include "../../Session/Game/Player.h"
#include "../../Session/FhishiX/gameobject/GameObjectArgument.h"
struct MapInitDto {
    const std::vector<std::pair<std::string, int>>* syncObjectsRef = nullptr;

    void Clear() {
        syncObjectsRef = nullptr;
    }

    size_t GetDtoBinaryLength() const {
        size_t len = 1; // 헤더 (SocketEventType)

        // 동적 오브젝트 정보 길이 계산
        len += 2; // objCount (2 byte)
        if (syncObjectsRef != nullptr) {
            for (const auto& pair : *syncObjectsRef) {
                len += sizeof(int) + sizeof(uint16_t) + pair.first.length(); // targetId(4) + nameLen(2) + name(N)
            }
        }
        return len;
    }

    void ToBinary(uint8_t* buffer) const {
        size_t offset = 0;
        buffer[offset++] = static_cast<uint8_t>(SocketEventType::MapInit);

        // 동적 오브젝트 직렬화
        uint16_t objCount = syncObjectsRef != nullptr ? static_cast<uint16_t>(syncObjectsRef->size()) : 0;
        std::memcpy(buffer + offset, &objCount, sizeof(objCount));
        offset += sizeof(objCount);

        if (syncObjectsRef != nullptr) {
            for (const auto& pair : *syncObjectsRef) {
                // 1. Target ID (4 bytes)
                std::memcpy(buffer + offset, &pair.second, sizeof(pair.second));
                offset += sizeof(pair.second);

                // 2. Name Length (2 bytes)
                uint16_t nameLen = static_cast<uint16_t>(pair.first.length());
                std::memcpy(buffer + offset, &nameLen, sizeof(nameLen));
                offset += sizeof(nameLen);

                // 3. Name String (N bytes)
                std::memcpy(buffer + offset, pair.first.c_str(), nameLen);
                offset += nameLen;
            }
        }
    }
};

#endif //FPSPROJECTSERVER_MAPINITDTO_H