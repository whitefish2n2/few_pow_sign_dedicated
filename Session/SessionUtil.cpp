//
// Created by white on 25. 10. 14.
//

#include "SessionUtil.h"
Player *SessionUtil::GetPlayerFromPeer(const ENetPeer &peer) {
    return static_cast<Player *>(peer.data);
}
