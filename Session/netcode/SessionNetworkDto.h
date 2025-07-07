//
// Created by user on 25. 4. 24.
//

#ifndef PLAYERNETWORKDTOS_H
#define PLAYERNETWORKDTOS_H
#include <string>

#include "../Game/Player.h"
#include "../Dto/SessionStatus.h"
#include "../FhishiX/Vector3.h"
#include <nlohmann/json.hpp>

#include "DedicatedNewPlayerDto.h"
#include "../Dto/GameMode.h"

class GameSession;
NLOHMANN_JSON_SERIALIZE_ENUM(SESSIONSTATUS, {
                             {idle, "idle"},
                             {playing, "playing"},
                             });
NLOHMANN_JSON_SERIALIZE_ENUM(GameMode,{
    {DeathMatch,"DeathMatch"},
    {OneVsOne,"OneVsOne"},
    {Custom,"Custom"}
})
struct player_dto {
    std::string id;
    std::string name;
    uint8_t team;
    int kill = 0;
    int death = 0;
};
player_dto playerToPlayerDto(const Player& p);

struct GameSessionDto {
    std::string sessionId;
    SESSIONSTATUS status;
    std::vector<player_dto> players;
};

GameSessionDto getGameSessionDto(GameSession& p);

void to_json(nlohmann::json& j, const GameSessionDto& p);
void to_json(nlohmann::json& j, const player_dto& p);

struct GameSetupBoddari {
    std::string gameId;
    std::vector<DedicatedNewPlayerDto> players;
    GameMode gameMode;
    std::string map;
    std::string playerConnectKey;
};
void from_json(const nlohmann::json &j, GameSetupBoddari &g);

struct PickElementDto{
    std::string characterId;
    std::string userId;
};
void from_json(const nlohmann::json &j, PickElementDto &c);

struct CharacterSetDto {
    std::string sessionId;
    std::vector<PickElementDto> elements;
};
void from_json(const nlohmann::json &j, CharacterSetDto &c);


#endif //PLAYERNETWORKDTOS_H
