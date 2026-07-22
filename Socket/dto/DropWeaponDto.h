#ifndef FPSPROJECTSERVER_DROPWEAPONDTO_H
#define FPSPROJECTSERVER_DROPWEAPONDTO_H
#pragma once
#include <cstdint>
#include <cstddef>

// P2S 드롭요청: 페이로드 없음 (현재 장착무기를 드롭)
struct DropWeaponDto {
    void Parse(const uint8_t* /*data*/, const size_t& /*size*/) {}
};
#endif //FPSPROJECTSERVER_DROPWEAPONDTO_H
