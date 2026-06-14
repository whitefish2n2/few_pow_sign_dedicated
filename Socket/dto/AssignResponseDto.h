//
// Created by white on 26. 6. 11..
//

#ifndef FPSPROJECTSERVER_ASSIGNRESPONSEDTO_H
#define FPSPROJECTSERVER_ASSIGNRESPONSEDTO_H
#include <cstring>

#include "SocketEventType.h"


struct PlayerIdentityInfo {
    uint8_t publicKey;   // 세션 내에서 사용할 1바이트짜리 짧은 ID
    std::string userId;    // 매칭 서버에서 넘어온 실제 유저 고유 ID (또는 캐릭터 이름)
};

struct AssignResponseDto {
    uint8_t myPublicKey; // 요청한 본인의 Public Key
    std::vector<PlayerIdentityInfo> otherPlayers; // 방에 있는 다른 유저 정보

    [[nodiscard]] size_t GetDtoBinaryLength() const {
        // [헤더 1] + [내 키 1] + [배열 크기 1] = 3 bytes
        size_t len = 3;
        for (const auto& info : otherPlayers) {
            // [퍼블릭키 1] + [문자열 길이 2] + [실제 문자열 바이트 N]
            len += 1 + 2 + info.userId.length();
        }
        return len;
    }

    void ToBinary(uint8_t* buffer) const {
        size_t offset = 0;
        buffer[offset++] = static_cast<uint8_t>(SocketEventType::AssignResponse);
        buffer[offset++] = myPublicKey;

        auto count = static_cast<uint8_t>(otherPlayers.size());
        buffer[offset++] = count;

        for (const auto& info : otherPlayers) {
            buffer[offset++] = info.publicKey;

            auto strLen = static_cast<uint16_t>(info.userId.length());
            std::memcpy(buffer + offset, &strLen, sizeof(strLen));
            offset += sizeof(strLen);

            if (strLen > 0) {
                std::memcpy(buffer + offset, info.userId.c_str(), strLen);
                offset += strLen;
            }
        }
    }
};


#endif //FPSPROJECTSERVER_ASSIGNRESPONSEDTO_H