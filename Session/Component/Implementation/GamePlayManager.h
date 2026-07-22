//
// Created by white on 26. 6. 1..
//

#ifndef FPSPROJECTSERVER_GAMEPLAYMANAGER_H
#define FPSPROJECTSERVER_GAMEPLAYMANAGER_H
#include <unordered_map>

#include "SynchronizedObject.h"
#include "../../InGamePhase.h"
#include "../Definition/Component.h"
#include "../Definition/ComponentArgument.h"

class GamePlayManager final : public Component<GamePlayManager>{
    public:
    static constexpr int WIN_SCORE = 10;

    InGamePhase currentPhase = InGamePhase::Initialize;
    float phaseTimer = 0.0f;
    float initTimeOut = 30.0f;
    std::unordered_map<int, int> teamScores;   // team -> 라운드 승수
    void Awake() override;
    void Start() override;
    void FixedUpdate() override;

    void AddSynchronizedObject(ComponentHandle<SynchronizedObject> obj);

    void UpdateInitialize();

    void UpdateLoading();

    void OnAllPlayersLoaded();

    void UpdatePrepare();

    void UpdateFighting();

    void UpdateClosing();

    void UpdateCleaning();

    void EnterLoadingPhase();

    void AbortGame();

    void EnterPreparePhase();

    void RespawnAllPlayers();

    void EnterFightingPhase();

    void EnterClosingPhase(uint8_t winningTeam);

    void CheckOneTeamRemain();

    void ChangePhase(InGamePhase phase);

    void ParseFromString(const std::string &arg) override;

    std::string syncObjectIdentify;

    private:
    int timeOutSecond = 10;
};
#endif //FPSPROJECTSERVER_GAMEPLAYMANAGER_H