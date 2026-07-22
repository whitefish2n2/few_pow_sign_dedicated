#ifndef FPSPROJECTSERVER_SWAPWEAPONDTO_H
#define FPSPROJECTSERVER_SWAPWEAPONDTO_H
#pragma once
#include <cstdint>
#include <cstddef>
#include <stdexcept>

// P2S 스왑요청: 방향 (1=up, 0=down) → WeaponInventory::SwapDir
struct SwapWeaponDto {
    uint8_t dir = 0;

    void Parse(const uint8_t* data, const size_t& size) {
        if (size < 1) throw std::runtime_error("SwapWeaponDto::Parse - payload too small");
        dir = data[0];
    }
};
#endif //FPSPROJECTSERVER_SWAPWEAPONDTO_H
