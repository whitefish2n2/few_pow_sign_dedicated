    //
// Created by user on 25. 4. 24.
//
#pragma once
#include <string>
#include <enet/enet.h>

#include "../FhishiX/vector/Vector2.h"
#include "PlayerStatus.h"
#include "../Component/Definition/ComponentHandle.h"
#include "../FhishiX/gameobject/GameObject.h"
#include "../Component/Implementation/PlayerComponent.h"

class Player {
    public:
    std::string userId;
    std::string userName;
    std::string assignKey;
    uint64_t privateKey{};
    uint8_t publicKey{};
    playerStatus status;

    ComponentHandle<PlayerComponent> playerComponent;
    //Component<Player>

    ENetPeer* peer=nullptr;

    void SetCharacter(std::string characterId);
    Player() = default;
    Player(std::string id, std::string name, std::string assignKey, uint64_t privateKey, uint8_t publicKey, playerStatus status );
};
