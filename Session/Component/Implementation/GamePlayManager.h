//
// Created by white on 26. 6. 1..
//

#ifndef FPSPROJECTSERVER_GAMEPLAYMANAGER_H
#define FPSPROJECTSERVER_GAMEPLAYMANAGER_H
#include "SynchronizedObject.h"
#include "../../InGamePhase.h"
#include "../Definition/Component.h"
#include "../Definition/ComponentArgument.h"

class GamePlayManager final : public Component<GamePlayManager>{
    public:
    InGamePhase currentPhase = InGamePhase::Initialize;
    float phaseTimer = 0.0f;
    float initTimeOut = 30.0f;
    void Awake() override;
    void Start() override;
    void Update() override;

    void AddSynchronizedObject(ComponentHandle<SynchronizedObject> obj);

    void UpdateInitialize();

    void UpdateLoading();

    void UpdatePrepare();

    void UpdateFighting();

    void UpdateClosing();

    void EnterLoadingPhase();

    void AbortGame();

    void EnterPreparePhase();

    void EnterFightingPhase();

    void ChangePhase(InGamePhase phase);

    void ParseFromString(const std::string &arg) override;

    std::string syncObjectIdentify;

    private:
    int timeOutSecond = 10;
};
#endif //FPSPROJECTSERVER_GAMEPLAYMANAGER_H