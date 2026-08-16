#include "SessionNetworkDto.h"

#include "../GameSession.h"
#include "DedicatedNewPlayerDto.h"
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
player_dto playerToPlayerDto(const Player& p) {
    return player_dto{
        .id = p.userId,
        .name = p.userName,
        .team = p.status.team,
        .kill = p.status.kill,
        .death = p.status.death,
   };
}
GameSessionDto getGameSessionDto(GameSession& p) {
    std::vector<player_dto> newPlayers;
    for (const auto& e: (*p.players | std::views::values)) {
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
    if (mode == "FiveVsFive") return GameMode::FiveVsFive;
    throw std::invalid_argument("Unknown game mode: " + mode);
}
void from_json(const nlohmann::json& j, DedicatedNewPlayerDto& p) {
    p.id = j["id"];
    p.name = j["name"];
    p.key = j["key"];
    p.team = j["team"];
    p.characterId = j.value("characterId", std::string{});
}

void from_json(const nlohmann::json& j, GameSetupBoddari& g) {
    g.gameId = j.at("gameId").get<std::string>();
    g.players = j.at("players").get<std::vector<DedicatedNewPlayerDto>>();
    g.gameMode = parseGameMode(j.at("gameMode").get<std::string>());
    g.mapId = j.at("map").get<std::uint32_t>();
}

void from_json(const nlohmann::json &j, CharacterSetDto &c) {
    c.sessionId = j["sessionId"];
    c.elements = j["elements"];
}
void from_json(const nlohmann::json &j, PickElementDto &g) {
    g.characterId = j["characterId"];
    g.userId = j["userId"];
}   

