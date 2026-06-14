//
// Created by user on 25. 4. 24.
//
#pragma once


#ifndef PLAYERSTATUS_H
#define PLAYERSTATUS_H
#include "../Dto/NetworkStatus.h"
#include "../Dto/WEAPON.h"
#include "../FhishiX/vector/Vector3.h"
#include <vector>


struct playerStatus {

    int team = 0;

    int kill = 0;
    int death = 0;
    NetworkStatus networkStatus = not_connected;
    uint8_t loadingProgress = 0;
};
#endif //PLAYERSTATUS_H
