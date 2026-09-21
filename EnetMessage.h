//
// Created by white on 26. 9. 21..
//

#ifndef FPSPROJECTSERVER_ENETMESSAGE_H
#define FPSPROJECTSERVER_ENETMESSAGE_H
#include <array>
#include <cstdint>
#include "Constants.h"
struct SendTarget {
    ENetPeer *peer;
    enet_uint32 connectID;
};

struct EnetMessage{
    ENetPacket* packet = nullptr;
    uint8_t count = 0;
    std::array<SendTarget, Consts::MaxPlayers> targets;
};
#endif //FPSPROJECTSERVER_ENETMESSAGE_H