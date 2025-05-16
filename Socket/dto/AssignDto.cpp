//
// Created by white on 25. 5. 9.
//
#include "AssignDto.h"

void from_json(const nlohmann::json& j, AssignRequestDto& g) {
    g.UserId = j.at("userId").get<std::string>();
    g.SessionId = j.at("sessionId").get<std::string>();
    g.Key = j.at("key").get<std::string>();
}
void to_json(nlohmann::json& j, const AssignResponseDto& g)
{
    j["sessionKey"] = g.sessionKey;
    j["userPublicKey"] = g.userPublicKey;
    j["userSecretKey"] = g.userSecretKey;
}