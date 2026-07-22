//
// Created by white on 26. 6. 1..
//

#include "GamePlayManager.h"




#include "PlayerGenerateManager.h"
#include "../../GameSession.h"
#include "../../../ObjectPool.h"
#include "../../../Socket/dto/GenerateDto.h"
#include "../Definition/ComponentManager.h"
#include "../Implementation/SynchronizedObject.h"
#include "../../FhishiX/gameobject/GameObjectArgument.h"
#include "../Definition/ComponentFactory.h"
#include "../../../Socket/dto/MapInitDto.h"
#include "../../../Socket/dto/RespawnDto.h"
#include "../../../Socket/dto/PhaseChangeNotifyDto.h"
#include "../../../Socket/dto/GameEndNotifyDto.h"
#include "../../../Socket/dto/RoundEndNotifyDto.h"
#include <unordered_set>

void GamePlayManager::Awake() {
    ComponentArgument::Awake();
}

void GamePlayManager::Start() {
    ComponentArgument::Start();
    phaseTimer = 0.0f;
}

void GamePlayManager::AddSynchronizedObject(ComponentHandle<SynchronizedObject> obj) {
    std::pair<std::string,int> val = std::pair<std::string,int>(obj->gameObject->name, static_cast<int>(obj->gameObject.targetId));
    gameSession->objectNameToIdCahce.push_back(val);
}

void GamePlayManager::FixedUpdate() {
    // 타이머는 여기서 무조건 누적 (각 페이즈 함수에서 체크함)
    phaseTimer += gameSession->time.DeltaTime;

    switch (currentPhase) {
        case InGamePhase::Initialize: UpdateInitialize(); break;
        case InGamePhase::Prepare:    UpdatePrepare();    break;
        case InGamePhase::Fighting:   UpdateFighting();   break;
        case InGamePhase::Closing:    UpdateClosing();    break;
        case InGamePhase::Loading: UpdateLoading(); break;
        case InGamePhase::Cleaning: UpdateCleaning(); break;
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
        OnAllPlayersLoaded();
    } else if (phaseTimer >= 60.0f) { // 60초 로딩 타임아웃
        AbortGame();
    }
}
void GamePlayManager::OnAllPlayersLoaded() {
    std::cout << "[Game] 전원 로딩 100% 완료! 물리 객체 소환 및 동기화 시작." << std::endl;

    ComponentHandle<PlayerGenerateManager> playerGen = this->gameSession->componentManager->FindFirstComponent<PlayerGenerateManager>();

    gameSession->objectNameToIdCahce.clear();
    for (auto& syncObj : *gameSession->componentManager->GetOrCreatePool<SynchronizedObject>()) {
        if (!syncObj.gameObject) continue;
        syncObj.resendTicks = 30;   // 초기 상태 플러시: ~1초간 강제 방송
        gameSession->objectNameToIdCahce.emplace_back(
            syncObj.gameObject->name, static_cast<int>(syncObj.gameObject.targetId));
    }

    // ===== GeneratePlayer 브로드캐스트 =====
    if (!playerGen.isNull()) {
        playerGen->SummonPlayer();

        GenerateDto* rawGenDto = ObjectPool<GenerateDto>::GetInstance().Acquire();
        rawGenDto->Clear();

        for (auto& p : std::views::values(*gameSession->players)) {
            if (p.playerComponent.isNull()) continue; // 스폰 실패(스폰포인트 없음 등) 제외
            GenerateEntry entry;
            entry.publicKey   = p.publicKey;
            entry.team        = static_cast<uint8_t>(p.status.team);
            entry.characterId = p.status.characterId;
            entry.spawnPos    = p.playerComponent->GetGameObject()->transform.GetPosition();
            rawGenDto->players.push_back(std::move(entry));
        }

        auto genDto = std::unique_ptr<GenerateDto, void(*)(GenerateDto*)>(
            rawGenDto,
            [](GenerateDto* p) { ObjectPool<GenerateDto>::GetInstance().Release(p); }
        );

        BroadCastEvent* rawGenEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
        rawGenEvent->type = SocketEventType::GeneratePlayer;
        rawGenEvent->payload = std::move(genDto);
        rawGenEvent->target.clear(); // 전원 브로드캐스트

        std::shared_ptr<BroadCastEvent> genEvent(
            rawGenEvent,
            [](BroadCastEvent* p) {
                p->payload = nullptr;
                ObjectPool<BroadCastEvent>::GetInstance().Release(p);
            }
        );
        gameSession->BroadcastEvent(genEvent);
    }

    // ===== MapInit =====
    MapInitDto* rawInitDto = ObjectPool<MapInitDto>::GetInstance().Acquire();
    rawInitDto->Clear();

    rawInitDto->syncObjectsRef = &gameSession->objectNameToIdCahce;

    auto initDto = std::unique_ptr<MapInitDto, void(*)(MapInitDto*)>(
        rawInitDto,
        [](MapInitDto* p) { ObjectPool<MapInitDto>::GetInstance().Release(p); }
    );

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
}
void GamePlayManager::UpdatePrepare() {
    if (phaseTimer >= 30.0f) {
        ChangePhase(InGamePhase::Fighting);
        EnterFightingPhase();
    }
}
void GamePlayManager::UpdateFighting() {
    // 승패조건(팀 전멸)은 CheckOneTeamRemain()에서 Death 이벤트마다 체크(GameSession::IHitValidator가 호출) — 여기선 무승부/정체 방지용 타임아웃만
    if (phaseTimer >= 180.0f) {
        ChangePhase(InGamePhase::Prepare);
        RespawnAllPlayers();
        EnterPreparePhase();
    }
}
void GamePlayManager::UpdateClosing() {
    // EnterClosingPhase 진입 시점에 이미 GameEndNotify 방송+집계 끝남 — 여긴 클라가 결과화면 볼 시간만 벌어줌
    if (phaseTimer >= 10.0f) {
        ChangePhase(InGamePhase::Cleaning);
    }
}

void GamePlayManager::UpdateCleaning() {
    gameSession->running = false;   // 자기 스레드라 join 불가 → 루프 자연 종료, 수확은 리퍼가 (CheckAllPlayerDisconnected와 동일 패턴)
}

void GamePlayManager::EnterLoadingPhase() {
    std::cout << "[Game] Loading Phase Started!." << std::endl;
}

void GamePlayManager::AbortGame() {
    std::cout<<"[Game} Aborted Game!" << std::endl;
    //todo: 게임 닷지 메시지 발송
}

void GamePlayManager::RespawnAllPlayers() {
    ComponentHandle<PlayerGenerateManager> playerGen = this->gameSession->componentManager->FindFirstComponent<PlayerGenerateManager>();
    if (playerGen.isNull()) return;

    RespawnDto* rawDto = ObjectPool<RespawnDto>::GetInstance().Acquire();
    rawDto->Clear();

    for (auto& p : std::views::values(*gameSession->players)) {
        if (p.playerComponent.isNull()) continue;

        Vector3 pos = playerGen->GetNextSpawnPosition(p.status.team);
        p.playerComponent->GetGameObject()->transform.SetPosition(pos);

        if (!p.playerComponent->IsAlive()) {
            p.playerComponent->ResetLife();
            p.playerComponent->GetGameObject()->SetActive(true);   // Death()에서 비활성화됐던 것 복구(무기는 이미 Death 시점에 드롭됨)
        }

        RespawnEntry entry;
        entry.publicKey = p.publicKey;
        entry.position  = pos;
        rawDto->players.push_back(entry);
    }

    auto dto = std::unique_ptr<RespawnDto, void(*)(RespawnDto*)>(
        rawDto, [](RespawnDto* p) { ObjectPool<RespawnDto>::GetInstance().Release(p); });

    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
    rawEvent->type = SocketEventType::RespawnPlayer;
    rawEvent->payload = std::move(dto);
    rawEvent->target.clear();
    std::shared_ptr<BroadCastEvent> event(rawEvent, [](BroadCastEvent* p) {
        p->payload = nullptr;
        ObjectPool<BroadCastEvent>::GetInstance().Release(p);
    });
    gameSession->BroadcastEvent(event);
}

void GamePlayManager::EnterPreparePhase() {
    std::cout << "[Game] Prepare Phase Started!." << std::endl;

    PhaseChangeNotifyDto* rawDto = ObjectPool<PhaseChangeNotifyDto>::GetInstance().Acquire();
    rawDto->phase    = static_cast<uint8_t>(InGamePhase::Prepare);
    rawDto->duration = 30.0f;   // UpdatePrepare()의 30.0f와 동일하게 유지

    auto dto = std::unique_ptr<PhaseChangeNotifyDto, void(*)(PhaseChangeNotifyDto*)>(
        rawDto, [](PhaseChangeNotifyDto* p) { ObjectPool<PhaseChangeNotifyDto>::GetInstance().Release(p); });

    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
    rawEvent->type = SocketEventType::PhaseChangeNotify;
    rawEvent->payload = std::move(dto);
    rawEvent->target.clear();
    std::shared_ptr<BroadCastEvent> event(rawEvent, [](BroadCastEvent* p) {
        p->payload = nullptr;
        ObjectPool<BroadCastEvent>::GetInstance().Release(p);
    });
    gameSession->BroadcastEvent(event);
}

void GamePlayManager::EnterFightingPhase() {

    std::cout << "[Game] Fighting Phase Started!" << std::endl;

    PhaseChangeNotifyDto* rawDto = ObjectPool<PhaseChangeNotifyDto>::GetInstance().Acquire();
    rawDto->phase    = static_cast<uint8_t>(InGamePhase::Fighting);
    rawDto->duration = 180.0f;   // UpdateFighting()의 180.0f와 동일하게 유지

    auto dto = std::unique_ptr<PhaseChangeNotifyDto, void(*)(PhaseChangeNotifyDto*)>(
        rawDto, [](PhaseChangeNotifyDto* p) { ObjectPool<PhaseChangeNotifyDto>::GetInstance().Release(p); });

    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
    rawEvent->type = SocketEventType::PhaseChangeNotify;
    rawEvent->payload = std::move(dto);
    rawEvent->target.clear();
    std::shared_ptr<BroadCastEvent> event(rawEvent, [](BroadCastEvent* p) {
        p->payload = nullptr;
        ObjectPool<BroadCastEvent>::GetInstance().Release(p);
    });
    gameSession->BroadcastEvent(event);
}

void GamePlayManager::EnterClosingPhase(uint8_t winningTeam) {
    std::cout << "[Game] Closing Phase Started! Winning Team: " << static_cast<int>(winningTeam) << std::endl;

    // 호출 지점: UpdateFighting()의 승패조건(TODO) 확정 시 ChangePhase(InGamePhase::Closing) 와 함께 호출
    GameEndNotifyDto* rawDto = ObjectPool<GameEndNotifyDto>::GetInstance().Acquire();
    rawDto->winningTeam = winningTeam;

    auto dto = std::unique_ptr<GameEndNotifyDto, void(*)(GameEndNotifyDto*)>(
        rawDto, [](GameEndNotifyDto* p) { ObjectPool<GameEndNotifyDto>::GetInstance().Release(p); });

    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
    rawEvent->type = SocketEventType::GameEndNotify;
    rawEvent->payload = std::move(dto);
    rawEvent->target.clear();
    std::shared_ptr<BroadCastEvent> event(rawEvent, [](BroadCastEvent* p) {
        p->payload = nullptr;
        ObjectPool<BroadCastEvent>::GetInstance().Release(p);
    });
    gameSession->BroadcastEvent(event);
}

void GamePlayManager::CheckOneTeamRemain() {
    if (currentPhase != InGamePhase::Fighting) return;

    std::unordered_set<int> aliveTeams;
    for (const auto& p : std::views::values(*gameSession->players)) {
        if (p.playerComponent.isNull()) continue;
        if (p.playerComponent->IsAlive()) aliveTeams.insert(p.status.team);
    }

    if (aliveTeams.size() != 1) return;   // 0(동시전멸) 또는 2팀 이상 생존 → 라운드 계속

    int winningTeam = *aliveTeams.begin();
    int newScore = ++teamScores[winningTeam];

    RoundEndNotifyDto* rawDto = ObjectPool<RoundEndNotifyDto>::GetInstance().Acquire();
    rawDto->winningTeam      = static_cast<uint8_t>(winningTeam);
    rawDto->winningTeamScore = static_cast<uint8_t>(newScore);

    auto dto = std::unique_ptr<RoundEndNotifyDto, void(*)(RoundEndNotifyDto*)>(
        rawDto, [](RoundEndNotifyDto* p) { ObjectPool<RoundEndNotifyDto>::GetInstance().Release(p); });

    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
    rawEvent->type = SocketEventType::RoundEndNotify;
    rawEvent->payload = std::move(dto);
    rawEvent->target.clear();
    std::shared_ptr<BroadCastEvent> event(rawEvent, [](BroadCastEvent* p) {
        p->payload = nullptr;
        ObjectPool<BroadCastEvent>::GetInstance().Release(p);
    });
    gameSession->BroadcastEvent(event);

    if (newScore >= WIN_SCORE) {
        ChangePhase(InGamePhase::Closing);
        EnterClosingPhase(static_cast<uint8_t>(winningTeam));
    } else {
        ChangePhase(InGamePhase::Prepare);
        RespawnAllPlayers();
        EnterPreparePhase();
    }
}

void GamePlayManager::ChangePhase(InGamePhase phase) {
    phaseTimer = 0.0f;
    currentPhase = phase;
}
void GamePlayManager::ParseFromString(const std::string &arg) {
    ComponentArgument::ParseFromString(arg);
}

REGISTER_COMPONENT(GamePlayManager)

