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


    // 2. InputVector (2 bytes)
    this->InputVector.x = static_cast<int8_t>(data[0]);
    this->InputVector.y = static_cast<int8_t>(data[1]);

    // 3. RotationVector (12 bytes)
    std::memcpy(&this->inputPitch, &data[2], sizeof(float));
    std::memcpy(&this->inputYaw, &data[6], sizeof(float));
}
