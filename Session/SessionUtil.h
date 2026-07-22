//
// Created by white on 25. 10. 14.
//

#ifndef SESSIONUTIL_H
#define SESSIONUTIL_H
#include <algorithm>
#include <map>
#include <ranges>

#include "Game/Player.h"


class SessionUtil {
    public:
        static Player* GetPlayerFromPeer(ENetPeer *peer) {
            if (peer->data != nullptr)
                return static_cast<Player *>(peer->data);
            else return nullptr;
        }
    static float GetRewindOffsetSeconds(Player* player) {
            constexpr float kMaxRewindSeconds = 0.5f;
            if (player == nullptr || player->peer == nullptr) return 0.0f;

            float oneWaySeconds = static_cast<float>(player->peer->roundTripTime) / 1000.0f / 2.0f;
            return (std::min)(oneWaySeconds, kMaxRewindSeconds);
        }

};



#endif //SESSIONUTIL_H
