//
// Created by white on 25. 5. 11.
//

#include "MoveDto.h"

#include <stdexcept>

///
/// @param data enet packet payload without serverId
/// @param size
/// @return
void MoveDto::Parse(const uint8_t *data, const size_t &size)
{
    if (size < 18) {
        throw std::runtime_error("MoveDto::Parse - payload size too small");
    }

    // 1. 0~7번 바이트: 8바이트 통째로 읽어서 uint64_t로 취급
    std::memcpy(&this->UserSecretKey, &data[0], sizeof(uint64_t));

    // 2. InputVector (2 bytes)
    this->InputVector.x = static_cast<int8_t>(data[8]);
    this->InputVector.y = static_cast<int8_t>(data[9]);

    // 3. RotationVector (12 bytes)
    std::memcpy(&this->inputPitch, &data[10], sizeof(float));
    std::memcpy(&this->inputYaw, &data[14], sizeof(float));
}
