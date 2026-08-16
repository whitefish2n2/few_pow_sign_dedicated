#include "HitStructureDto.h"

#include <cstring>
#include <stdexcept>

void HitStructureDto::Parse(const uint8_t* data, const size_t& size) {
    if (size < 28) {
        throw std::runtime_error("HitStructureDto::Parse - payload size too small");
    }

    std::memcpy(&targetObjectId, &data[0], sizeof(uint32_t));
    std::memcpy(&origin, &data[4], sizeof(Vector3));
    std::memcpy(&dir, &data[16], sizeof(Vector3));
}
