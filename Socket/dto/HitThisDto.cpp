#include "HitThisDto.h"

#include <cstring>
#include <stdexcept>

void HitThisDto::Parse(const uint8_t* data, const size_t& size) {
    if (size < 25) {
        throw std::runtime_error("HitThisDto::Parse - payload size too small");
    }

    targetPublicKey = data[0];
    std::memcpy(&origin, &data[1], sizeof(Vector3));
    std::memcpy(&dir, &data[13], sizeof(Vector3));
}
