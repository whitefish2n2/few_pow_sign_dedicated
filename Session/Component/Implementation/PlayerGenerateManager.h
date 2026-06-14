//
// Created by white on 26. 6. 4..
//

#ifndef FPSPROJECTSERVER_PLAYERGENERATEMANAGER_H
#define FPSPROJECTSERVER_PLAYERGENERATEMANAGER_H
#include "CharacterSummonPosition.h"
#include "../Definition/ComponentArgument.h"
#include "../Definition/ComponentHandle.h"


class PlayerGenerateManager final : public Component<PlayerGenerateManager> {
    public:
    std::vector<ComponentHandle<CharacterSummonPosition>> summonPositions;
    std::unordered_map<int, std::vector<ComponentHandle<CharacterSummonPosition>>> teamSpawnPoints;
    std::unordered_map<int, int> nextSpawnIndex;
    void Start() override;

    void SummonPlayer();
};


#endif //FPSPROJECTSERVER_PLAYERGENERATEMANAGER_H