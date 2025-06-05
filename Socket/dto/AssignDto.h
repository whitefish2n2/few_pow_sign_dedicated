//
// Created by white on 25. 5. 9.
//

#ifndef ASSIGNDTO_H
#define ASSIGNDTO_H
#include <string>
#include <nlohmann/json.hpp>

class AssignRequestDto
{
    public:
    std::string UserId;
    std::string SessionId;
    std::string Key;

    static AssignRequestDto Parse(const uint8_t* data, const size_t& size);
};
struct AssignResponseDto
{
    std::uint16_t sessionKey;
    std::uint64_t userSecretKey;
    std::uint8_t userPublicKey;
};

void to_json(nlohmann::json& j, const AssignResponseDto& d);
void from_json(const nlohmann::json& j, AssignRequestDto& g);


#endif //ASSIGNDTO_H
