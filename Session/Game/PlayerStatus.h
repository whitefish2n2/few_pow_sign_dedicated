//
// Created by user on 25. 4. 24.
//
#pragma once


#ifndef PLAYERSTATUS_H
#define PLAYERSTATUS_H
#include "../Dto/NetworkStatus.h"
#include "../Dto/Weapons.h"
#include "../FhishiX/Vector3.h"
#include <vector>


struct player_status {
    uint8_t team;

    int kill;
    int death;

    Vector3 position;
    Vector3 velocity;

    Vector3 rotation;
    WEAPONS holdingWeapon;
    std::vector<WEAPONS> weapons = std::vector<WEAPONS>();
    NetworkStatus networkStatus;
};
#endif //PLAYERSTATUS_H
