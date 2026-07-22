#ifndef FPSPROJECTSERVER_INTERACTDTO_H
#define FPSPROJECTSERVER_INTERACTDTO_H
#pragma once
#include <cstdint>
#include <cstddef>

// P2S 상호작용요청: 페이로드 없음. 서버가 플레이어 근접 검색으로 대상을 권위적으로 판정
// (무기 근처면 픽업 → GetWeaponNotify). 클라는 오브젝트 id를 보내지 않음.
struct InteractDto {
    void Parse(const uint8_t* /*data*/, const size_t& /*size*/) {}
};
#endif //FPSPROJECTSERVER_INTERACTDTO_H
