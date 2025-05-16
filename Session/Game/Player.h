//
// Created by user on 25. 4. 24.
//
#pragma once
#include <string>
#include <enet/enet.h>

#include "../FhishiX/Vector2.h"
#include "PlayerStatus.h"
class Player {
    public:
    std::string id;
    std::string name;
    std::string assignKey;
    uint16_t privateKey;
    uint8_t publicKey;
    player_status status;

    ENetPeer* peer=nullptr;

    void Move(Vector2<int> inputVector);
    void Rotate(Vector3 r);
    void Jump();
    Player() = default;
    Player(std::string id, std::string name, std::string assignKey, uint16_t privateKey, uint8_t publicKey, player_status status );
};
