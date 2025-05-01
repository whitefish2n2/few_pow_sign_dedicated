//
// Created by user on 25. 4. 24.
//
#pragma once
#ifndef PLAYER_H
#define PLAYER_H
#include <string>
#include "PlayerStatus.h"
struct player {
    std::string id;
    std::string name;
    uint16_t privateKey;
    uint8_t publicKey;
    player_status status;
    player(std::string name, uint16_t privateKey, uint8_t publicKey, player_status status );
};
#endif //PLAYER_H
