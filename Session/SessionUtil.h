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
            return static_cast<Player *>(peer->data);
        }

        static bool ContainsPrivateKey(std::map<uint64_t,Player>& list, uint64_t key) {
            for (const Player& v: list|std::views::values) {
                if (v.privateKey == key) return true;
            }
            return false;
        }
};



#endif //SESSIONUTIL_H
