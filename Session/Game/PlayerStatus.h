//
// Created by user on 25. 4. 24.
//
#pragma once


#ifndef PLAYERSTATUS_H
#define PLAYERSTATUS_H
#include <complex.h>

#include "../Dto/NetworkStatus.h"
#include "../Dto/WEAPON.h"
#include "../FhishiX/vector/Vector3.h"
#include <vector>

#include "LifeState.h"


struct playerStatus {

    int team = 0;
    uint8_t characterId = 0;
    int kill = 0;
    int death = 0;
    NetworkStatus networkStatus = not_connected;
    LifeState lifeState = LifeState::Alive;
    uint8_t loadingProgress = 0;
};
#endif //PLAYERSTATUS_H
