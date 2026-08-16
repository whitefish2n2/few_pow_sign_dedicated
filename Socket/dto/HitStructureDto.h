#ifndef FPSPROJECTSERVER_HITSTRUCTUREDTO_H
#define FPSPROJECTSERVER_HITSTRUCTUREDTO_H
#pragma once
#include <cstdint>
#include <cstddef>

#include "../../Session/FhishiX/vector/Vector3.h"

// P2S 명중클레임: 플레이어가 아닌 구조물(GameObject id로 지칭)을 어디서(origin) 어느 방향(dir)으로 맞췄다고 주장하는지
class HitStructureDto {
public:
    uint32_t targetObjectId = 0;
    Vector3 origin;
    Vector3 dir;
    void Parse(const uint8_t* data, const size_t& size);
};
#endif //FPSPROJECTSERVER_HITSTRUCTUREDTO_H
