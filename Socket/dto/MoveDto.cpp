//
// Created by white on 25. 5. 11.
//

#include "MoveDto.h"

#include <stdexcept>

///
/// @param data enet packet payload without serverId
/// @param size
/// @return
MoveDto MoveDto::Parse(const uint8_t* data, const size_t& size)
{
    if (size < 25) {
        throw std::runtime_error("MoveDto::Parse - payload size too small");
    }
    MoveDto result = MoveDto();
    std::memcpy(&result.UserSecretKey, &data[0], sizeof(uint16_t));
    result.InputVector.x = static_cast<int8_t>(data[2]);
    result.InputVector.y = static_cast<int8_t>(data[3]);
    std::memcpy(&(result.RotationVector).x, &data[4], sizeof(float));
    std::memcpy(&(result.RotationVector).y, &data[8], sizeof(float));
    std::memcpy(&(result.RotationVector).z, &data[12], sizeof(float));
    std::memcpy(&(result.RotatedVector).x, &data[16], sizeof(float));
    std::memcpy(&(result.RotatedVector).y, &data[20], sizeof(float));
    std::memcpy(&(result.RotatedVector).z, &data[24], sizeof(float));
    return result;
}
