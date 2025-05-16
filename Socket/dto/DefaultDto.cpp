//
// Created by white on 25. 5. 11.
//

#include "DefaultDto.h"

#include <vector>

DefaultDto DefaultDto::Parse(const uint8_t*data, const size_t& size)
{
    DefaultDto result = DefaultDto();
    std::memcpy(&result.UserSecretKey, &data[0], sizeof(uint16_t));
    result.payload = std::vector<uint8_t>(data+sizeof(uint16_t), data + size);
    return result;
}
