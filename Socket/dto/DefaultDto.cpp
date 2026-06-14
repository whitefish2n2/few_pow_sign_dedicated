//
// Created by white on 25. 5. 11.
//

#include "DefaultDto.h"

#include <vector>

void DefaultDto::Parse(const uint8_t *data, const size_t &size)
{
    std::memcpy(&this->UserSecretKey, &data[0], sizeof(uint16_t));
    this->payload.assign(data + sizeof(uint16_t), data + size);
}
