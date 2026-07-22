//
// Created by white on 26. 6. 4..
//

#include "../Implementation/PlayerGenerateManager.h"

#include "../../../PrefabSystem/PrefabManager.h"
#include "../Definition/ComponentFactory.h"

#include "./PlayerComponent.h"
#include "../../Game/data/CharacterRegistry.h"


void PlayerGenerateManager::Start() {
    ComponentArgument::Start();

}

void PlayerGenerateManager::SummonPlayer() {
    for (const auto& pos : *gameSession->componentManager->GetOrCreatePool<CharacterSummonPosition>()) {
        teamSpawnPoints[pos.teamId].push_back(pos.MakeHandle());
    }
    for (auto& p : std::views::values(*gameSession->players)) {
        int myTeam = p.status.team;

        auto it = teamSpawnPoints.find(myTeam);
        if (it == teamSpawnPoints.end() || it->second.empty()) {
            std::cout << "[Warning] Team " << myTeam << " 스폰 포인트가 없습니다!" << std::endl;
            continue;
        }

        auto& spawns = it->second;

        int index = nextSpawnIndex[myTeam] % spawns.size();
        auto selectedPosHandle = spawns[index];
        nextSpawnIndex[myTeam]++; // 다음 사람을 위해 인덱스 증가

        auto selectedPos = selectedPosHandle.operator->();
        const auto* info = CharacterRegistry::Get(p.status.characterId);
        auto obj = PrefabManager::Instantiate(info ? info->prefabId : 2u, gameSession);
        obj->transform.SetPosition(selectedPos->GetGameObject()->transform.GetPosition());
        p.playerComponent = obj->GetComponent<PlayerComponent>();
        p.playerComponent->publicKey = p.publicKey;
        p.SetCharacter(p.status.characterId);
    }
}

Vector3 PlayerGenerateManager::GetNextSpawnPosition(int team) {
    auto it = teamSpawnPoints.find(team);
    if (it == teamSpawnPoints.end() || it->second.empty()) {
        std::cout << "[Warning] Team " << team << " 스폰 포인트가 없습니다!" << std::endl;
        return Vector3::Zero();
    }

    auto& spawns = it->second;
    int index = nextSpawnIndex[team] % spawns.size();
    auto selectedPosHandle = spawns[index];
    nextSpawnIndex[team]++;

    return selectedPosHandle.operator->()->GetGameObject()->transform.GetPosition();
}

REGISTER_COMPONENT(PlayerGenerateManager)
