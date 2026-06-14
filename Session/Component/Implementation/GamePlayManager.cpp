//
// Created by white on 26. 6. 1..
//

#include "GamePlayManager.h"

#include "PlayerGenerateManager.h"
#include "../../GameSession.h"
#include "../../../ObjectPool.h"
#include "../Definition/ComponentManager.h"
#include "../Implementation/SynchronizedObject.h"
#include "../../FhishiX/gameobject/GameObjectArgument.h"
#include "../Definition/ComponentFactory.h"
#include "../../../Socket/dto/MapInitDto.h"

void GamePlayManager::Awake() {
    ComponentArgument::Awake();
}

void GamePlayManager::Start() {
    ComponentArgument::Start();
    for (const auto& syncObj: *gameSession->componentManager->GetOrCreatePool<SynchronizedObject>()) {
        std::pair<std::string,int> val = std::pair<std::string,int>(syncObj.gameObject->name, static_cast<int>(syncObj.gameObject.targetId));
        gameSession->objectNameToIdCahce.push_back(val);
    }
    phaseTimer = 0.0f;
}

void GamePlayManager::AddSynchronizedObject(ComponentHandle<SynchronizedObject> obj) {
    std::pair<std::string,int> val = std::pair<std::string,int>(obj->gameObject->name, static_cast<int>(obj->gameObject.targetId));
    gameSession->objectNameToIdCahce.push_back(val);
}

void GamePlayManager::Update() {
    // 타이머는 여기서 무조건 누적 (각 페이즈 함수에서 체크함)
    phaseTimer += gameSession->time.DeltaTime;

    switch (currentPhase) {
        case InGamePhase::Initialize: UpdateInitialize(); break;
        case InGamePhase::Prepare:    UpdatePrepare();    break;
        case InGamePhase::Fighting:   UpdateFighting();   break;
        case InGamePhase::Closing:    UpdateClosing();    break;
        case InGamePhase::Loading: UpdateLoading(); break;
        default: ;
    }
}


void GamePlayManager::UpdateInitialize() {
    bool allConnected = true;
    for (const auto& p : std::views::values(*gameSession->players)) {
        if (p.status.networkStatus != NetworkStatus::connected) {
            allConnected = false; break;
        }
    }

    if (allConnected) {
        ChangePhase(InGamePhase::Loading);
        EnterLoadingPhase();

    } else if (phaseTimer >= 30.0f) {
        AbortGame();
    }
}

void GamePlayManager::UpdateLoading() {
    bool allLoaded = true;
    for (const auto& p : std::views::values(*gameSession->players)) {
        if (p.status.loadingProgress < 100) {
            allLoaded = false;
            break;
        }
    }
    if (allLoaded) {
        std::cout << "[Game] 전원 로딩 100% 완료! 물리 객체 소환 및 동기화 시작." << std::endl;

        ComponentHandle<PlayerGenerateManager> playerGen = this->gameSession->componentManager->FindFirstComponent<PlayerGenerateManager>();

        if (!playerGen.isNull()) {
            playerGen->SummonPlayer();
        }

        // 2. ObjectPool에서 MapInitDto 가져오기
        MapInitDto* rawInitDto = ObjectPool<MapInitDto>::GetInstance().Acquire();
        rawInitDto->Clear();

        rawInitDto->syncObjectsRef = &gameSession->objectNameToIdCahce;

        auto initDto = std::unique_ptr<MapInitDto, void(*)(MapInitDto*)>(
            rawInitDto,
            [](MapInitDto* p) { ObjectPool<MapInitDto>::GetInstance().Release(p); }
        );

        // 4. BroadCastEvent 만들어서 쏘기
        BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
        rawEvent->type = SocketEventType::MapInit; // 혹은 DynamicInit
        rawEvent->payload = std::move(initDto);
        rawEvent->target.clear();

        std::shared_ptr<BroadCastEvent> event(
            rawEvent,
            [](BroadCastEvent* p) {
                p->payload = nullptr;
                ObjectPool<BroadCastEvent>::GetInstance().Release(p);
            }
        );

        gameSession->BroadcastEvent(event);

        ChangePhase(InGamePhase::Prepare);
        EnterPreparePhase();

    } else if (phaseTimer >= 60.0f) { // 60초 로딩 타임아웃
        AbortGame();
    }
}

void GamePlayManager::UpdatePrepare() {
    if (phaseTimer >= 30.0f) {
        ChangePhase(InGamePhase::Fighting);
        EnterFightingPhase();
    }
}
void GamePlayManager::UpdateFighting() {
    // TODO: 누군가 다 죽었는지(승패조건) 체크 로직 추가
    if (phaseTimer >= 180.0f) {
        ChangePhase(InGamePhase::Prepare);
        EnterPreparePhase();
    }
}
void GamePlayManager::UpdateClosing(){};

void GamePlayManager::EnterLoadingPhase() {
    std::cout << "[Game] Loading Phase Started!." << std::endl;
}

void GamePlayManager::AbortGame() {
    std::cout<<"[Game} Aborted Game!" << std::endl;
    //todo: 게임 닷지 메시지 발송
}

void GamePlayManager::EnterPreparePhase() {
    std::cout << "[Game] Prepare Phase Started!." << std::endl;
}

void GamePlayManager::EnterFightingPhase() {

    std::cout << "[Game] Fighting Phase Started!" << std::endl;
}

void GamePlayManager::ChangePhase(InGamePhase phase) {
    phaseTimer = 0.0f;
    currentPhase = phase;
}
void GamePlayManager::ParseFromString(const std::string &arg) {
    ComponentArgument::ParseFromString(arg);
}

REGISTER_COMPONENT(GamePlayManager)