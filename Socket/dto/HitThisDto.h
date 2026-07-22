#ifndef FPSPROJECTSERVER_HITTHISDTO_H
#define FPSPROJECTSERVER_HITTHISDTO_H
#pragma once
#include <cstdint>
#include <cstddef>

#include "../../Session/FhishiX/vector/Vector3.h"

// P2S 명중클레임: 누구를(targetPublicKey) 어디서(origin) 어느 방향(dir)으로 맞췄다고 주장하는지
class HitThisDto {
public:
    uint8_t targetPublicKey = 0;
    Vector3 origin;
    Vector3 dir;
    void Parse(const uint8_t* data, const size_t& size);
};
#endif //FPSPROJECTSERVER_HITTHISDTO_H
