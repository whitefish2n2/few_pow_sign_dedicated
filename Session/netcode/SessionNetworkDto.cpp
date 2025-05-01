#include "SessionNetworkDto.h"

#include "../GameSession.h"
#include "newPlayerDto.h"
#include "../../util/util.h"

void to_json(nlohmann::json& j, const player_dto& p) {
    j = nlohmann::json{
        {"id",p.id},
        {"name", p.name},
        {"team", p.team},
        {"kill", p.kill},
        {"death", p.death},
    };
}
void to_json(nlohmann::json& j, const GameSessionDto& p) {
    j = nlohmann::json{
        {"sessionId",p.sessionId},
        {"status",p.status},
        {"players",p.players}
    };
}
player_dto playerToPlayerDto(const player& p) {
    return player_dto{
        .id = p.id,
        .name = p.name,
        .team = p.status.team,
        .kill = p.status.kill,
        .death = p.status.death,
   };
}
GameSessionDto getGameSessionDto(GameSession& p) {
    std::vector<player_dto> newPlayers;
    for (player e: *(p.players)) {
        newPlayers.push_back(playerToPlayerDto(e));
    }
    return GameSessionDto{p.sessionId , p.status, newPlayers};
}
//parse
GameMode parseGameMode(const std::string& mode) {
    if (mode == "DeathMatch") return GameMode::DeathMatch;
    if (mode == "OneVsOne")   return GameMode::OneVsOne;
    if (mode == "Custom")     return GameMode::Custom;
    if (mode == "Solo")       return GameMode::Solo;
    throw std::invalid_argument("Unknown game mode: " + mode);
}
void from_json(const nlohmann::json& j, NewPlayerDto& p) {
    p.id = j["id"];
    p.name = j["name"];
}

void from_json(const nlohmann::json& j, GameSetupBoddari& g) {
    g.gameId = j.at("gameId").get<std::string>();
    g.players = j.at("players").get<std::vector<NewPlayerDto>>();
    g.gameMode = parseGameMode(j.at("gameMode").get<std::string>());
    g.map = j.at("map").get<std::string>();
}
