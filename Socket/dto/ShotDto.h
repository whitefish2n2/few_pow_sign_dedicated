#ifndef FPSPROJECTSERVER_SHOTDTO_H
#define FPSPROJECTSERVER_SHOTDTO_H
#pragma once
#include <cstdint>
#include <cstddef>

// P2S 발사의도: 페이로드 없음. 서버가 연사율/탄약 게이트 후 눈위치+조준으로 origin/dir 자체 구성
struct ShotDto {
    void Parse(const uint8_t* /*data*/, const size_t& /*size*/) {}
};
#endif //FPSPROJECTSERVER_SHOTDTO_H
