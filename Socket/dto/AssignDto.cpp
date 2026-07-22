//
// Created by white on 25. 5. 9.
//
#include "AssignDto.h"

#include <iostream>

void from_json(const nlohmann::json& j, AssignRequestDto& g) {
    g.UserId = j.at("UserId").get<std::string>();
    g.SessionId = j.at("SessionId").get<std::string>();
    g.Key = j.at("Key").get<std::string>();
}

void AssignRequestDto::Parse(const uint8_t *data, const size_t &size) {
    std::string message(reinterpret_cast<const char*>(data), size);

    nlohmann::json j = nlohmann::json::parse(message);

    from_json(j, *this);
}