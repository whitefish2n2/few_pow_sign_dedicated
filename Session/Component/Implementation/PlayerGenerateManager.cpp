//
// Created by white on 26. 6. 4..
//

#include "../Implementation/PlayerGenerateManager.h"

#include "../../../PrefabSystem/PrefabManager.h"
#include "../Definition/ComponentFactory.h"

#include "./PlayerComponent.h"


void PlayerGenerateManager::Start() {
    ComponentArgument::Start();
    for (const auto& pos : *gameSession->componentManager->GetOrCreatePool<CharacterSummonPosition>()) {
        teamSpawnPoints[pos.teamId].push_back(pos.MakeHandle());
    }
}

void PlayerGenerateManager::SummonPlayer() {
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
        auto obj = PrefabManager::Instantiate("playerPrefab-1", gameSession);//todo: 캐릭터에 맞는 프리팹 생성ㄴ
        obj->transform.SetPosition(selectedPos->GetGameObject()->transform.GetPosition());
        p.playerComponent = obj->GetComponent<PlayerComponent>();
    }
}

REGISTER_COMPONENT(PlayerGenerateManager)
