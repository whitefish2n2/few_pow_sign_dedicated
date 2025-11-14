//
// Created by user on 25. 4. 24.
//
#pragma once


#ifndef PLAYERSTATUS_H
#define PLAYERSTATUS_H
#include "../Dto/NetworkStatus.h"
#include "../Dto/WEAPON.h"
#include "../FhishiX/Vector/Vector3.h"
#include <vector>


struct player_status {
    uint8_t team = 0;

    int kill = 0;
    int death = 0;

    Vector3 position = Vector3::Zero();
    Vector3 velocity = Vector3::Zero();

    Vector3 rotation = Vector3::Zero();
    WEAPON holdingWeapon;
    std::vector<WEAPON> weapons = std::vector<WEAPON>();
    NetworkStatus networkStatus;
};
#endif //PLAYERSTATUS_H
