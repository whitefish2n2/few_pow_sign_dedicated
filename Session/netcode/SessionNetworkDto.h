//
// Created by user on 25. 4. 24.
//

#ifndef PLAYERNETWORKDTOS_H
#define PLAYERNETWORKDTOS_H
#include <string>

#include "../Game/Player.h"
#include "../Dto/SessionStatus.h"
#include "../FhishiX/vector/Vector3.h"
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
    int team;
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
    std::uint32_t mapId;
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

struct ServerStatusDto {
    double cpuUsagePercent;
    long long memoryUsageMB;
    int currentSessionCount;
    int maxSessionCount; // 서버가 버틸 수 있는 최대 세션 수
};

inline void to_json(nlohmann::json& j, const ServerStatusDto& dto) {
    j = nlohmann::json{
            {"cpuUsagePercent", dto.cpuUsagePercent},
            {"memoryUsageMB", dto.memoryUsageMB},
            {"currentSessionCount", dto.currentSessionCount},
            {"maxSessionCount", dto.maxSessionCount}
    };
}

#endif //PLAYERNETWORKDTOS_H
