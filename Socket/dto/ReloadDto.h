#ifndef FPSPROJECTSERVER_RELOADDTO_H
#define FPSPROJECTSERVER_RELOADDTO_H
#pragma once
#include <cstdint>
#include <cstddef>

// P2S 리로드요청: 페이로드 없음 (현재 장착무기 리로드)
struct ReloadDto {
    void Parse(const uint8_t* /*data*/, const size_t& /*size*/) {}
};
#endif //FPSPROJECTSERVER_RELOADDTO_H
