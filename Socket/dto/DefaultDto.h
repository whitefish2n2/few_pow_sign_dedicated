//
// Created by white on 25. 5. 11.
//

#ifndef DEFAULTDTO_H
#define DEFAULTDTO_H
#include <cstdint>
#include <memory>
#include <vector>


class DefaultDto {
    public:
    uint16_t UserSecretKey;
    std::vector<uint8_t> payload;
    size_t payloadLength;
    static DefaultDto Parse(const uint8_t* data, const size_t& size);
};



#endif //DEFAULTDTO_H
