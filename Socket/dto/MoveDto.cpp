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
    if (size < 20) {
        throw std::runtime_error("MoveDto::Parse - payload size too small");
    }
    MoveDto result = MoveDto();
    //
    std::memcpy(&result.UserSecretKey, &data[0], sizeof(uint64_t));
    result.InputVector.x = static_cast<int8_t>(data[8]);
    result.InputVector.y = static_cast<int8_t>(data[9]);
    std::memcpy(&(result.RotationVector).x, &data[10], sizeof(float));
    std::memcpy(&(result.RotationVector).y, &data[14], sizeof(float));
    std::memcpy(&(result.RotationVector).z, &data[18], sizeof(float));
    return result;
}
