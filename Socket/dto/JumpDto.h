#ifndef FPSPROJECTSERVER_JUMPDTO_H
#define FPSPROJECTSERVER_JUMPDTO_H
#pragma once
#include <cstdint>
#include <cstddef>

// P2S 점프요청: 페이로드 없음. 서버가 접지 검사로 권위 판정 후 임펄스 적용
struct JumpDto {
    void Parse(const uint8_t* /*data*/, const size_t& /*size*/) {}
};
#endif //FPSPROJECTSERVER_JUMPDTO_H
