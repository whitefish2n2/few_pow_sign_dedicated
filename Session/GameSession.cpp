#include "GameSession.h"

#ifdef _WIN64
#include <windows.h>
#endif
#include <iostream>
#include <enet/enet.h>
#include <thread>
#include <random>
#include <string>
#include <ranges>
#include <utility>

#include "Game/Player.h"
#include "SessionUtil.h"
#include "../Constants.h"
#include "../ServerStatics.h"
#include "../Socket/EnetClient.h"
#include "../Socket/dto/AssignDto.h"
#include "../Socket/dto/SocketEventType.h"
#include "../util/util.h"
#include "FhishiX/gameobject/GameObjectManager.h"
#include "Game/MapManager.h"
#include "Game/Map/MapConstructer/PhysicsSystemConstructor.h"
#include "../util/Log.h"
#include "Game/PhysicsSystem.h"
#include "../Socket/dto/BroadcastMoveDto.h"
#include "../Socket/dto/LoadingProgressDto.h"
#include "Component/Implementation/PlayerComponent.h"
#include "../Socket/dto/AssignResponseDto.h"
#include <type_traits>

#include "../ObjectPool.h"
#include "../PrefabSystem/PrefabManager.h"
#include "../Socket/dto/MapInitDto.h"
#include "../Socket/dto/GenerateDto.h"
#include "../Socket/dto/BroadcastMoveDto.h"
#include "../Socket/dto/BroadcastPlayerMoveDto.h"
#include "../Socket/dto/RespawnDto.h"
#include "../Socket/dto/HitDto.h"
#include "../Socket/dto/HitThisDto.h"
#include "../Socket/dto/HitStructureDto.h"
#include "../Socket/dto/DeathDto.h"
#include "../Socket/dto/GetWeaponNotifyDto.h"
#include "../Socket/dto/ReloadNotifyDto.h"
#include "../Socket/dto/SwapWeaponNotifyDto.h"
#include "../Socket/dto/ReloadNotifyDto.h"
#include "../Socket/dto/DropWeaponNotifyDto.h"
#include "../Socket/dto/PhaseChangeNotifyDto.h"
#include "../Socket/dto/GameEndNotifyDto.h"
#include "../Socket/dto/ShotNotifyDto.h"
#include "../Socket/dto/GenerateObjectDto.h"
#include "Component/Implementation/Interactable.h"
#include "Component/Implementation/WeaponInventory.h"
#include "../Socket/dto/SwapWeaponDto.h"
#include "Component/Implementation/SynchronizedObject.h"
#include "Game/data/WeaponRegistry.h"
#include "Component/Implementation/GamePlayManager.h"
#include "../Socket/dto/RoundEndNotifyDto.h"
#include "FhishiX/gameobject/rigidBody/Rigidbody.h"

GameSession::GameSession() {
    objectManager = std::make_unique<GameObjectManager>();
    objectManager->ownerSession = this;
    componentManager = std::make_unique<ComponentManager>();
    componentManager->ownerSession = this;
    physicsSystem = std::make_unique<PhysicsSystem>();
}
GameSession::~GameSession() {
    LOG_INFO("server destroyed");
    Stop();
}


void GameSession::ProcessEventQueue() {
    GameEventPtr e(nullptr, nullptr);
    while (eventQueue.try_dequeue(e)) {
        if (!this->running or !isRunning.load()) break;

        try {
            switch (e->type) {
                case SocketEventType::Assign: {
                    using AssignDtoPtr = std::unique_ptr<AssignRequestDto, void(*)(AssignRequestDto*)>;
                    auto* v = std::get_if<AssignDtoPtr>(&e->payload);
                    if (v == nullptr) break;
                    LOG_DEBUG("assign dto ptr로 변환 성공");

                    AssignRequestDto* dto = v->get();

                    for (auto& val : *players | std::views::values) {
                        if (val.assignKey == dto->Key) {
                            LOG_DEBUG("assign respnose dto 생성 시작");
                            val.peer = e->peer;
                            val.peerConnectId = e->peer->connectID;
                            e->peer->data = &val;
                            val.status.networkStatus = connected;


                            // 1. DTO 풀에서 가져오기
                            AssignResponseDto* rawResDto = ObjectPool<AssignResponseDto>::GetInstance().Acquire();
                            rawResDto->myPublicKey = val.publicKey;
                            rawResDto->otherPlayers.clear();

                            // 2. 방에 있는 다른 플레이어들의 정보 긁어오기
                            for (const auto& p : *players | std::views::values) {
                                if (p.publicKey != val.publicKey) { // 본인은 리스트에서 제외
                                    PlayerIdentityInfo info;
                                    info.publicKey = p.publicKey;
                                    info.userId = p.userId; // std::string
                                    rawResDto->otherPlayers.push_back(info);
                                }
                            }

                            // 3. 커스텀 딜리터 씌워서 스마트 포인터로 래핑
                            auto resDto = std::unique_ptr<AssignResponseDto, void(*)(AssignResponseDto*)>(
                                rawResDto,
                                static_cast<void(*)(AssignResponseDto*)>([](AssignResponseDto* p) {
                                    ObjectPool<AssignResponseDto>::GetInstance().Release(p);
                                })
                            );

                            // 4. BroadCastEvent 만들어서 타겟 지정 후 쏘기
                            BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
                            rawEvent->type = SocketEventType::AssignResponse;
                            rawEvent->payload = std::move(resDto);
                            rawEvent->target.clear();
                            rawEvent->target.push_back(val.peer);

                            std::shared_ptr<BroadCastEvent> event(
                                rawEvent,
                                [](BroadCastEvent* p) {
                                    p->payload = nullptr;
                                    ObjectPool<BroadCastEvent>::GetInstance().Release(p);
                                }
                            );

                            // 이 함수 안에서 자동으로 직렬화 후 EnqueueSend를 호출해 줍니다.
                            this->BroadcastEvent(event);
                            break;
                        }
                    }
                    break;
                }

                case SocketEventType::Move: {
                    using MoveDtoPtr = std::unique_ptr<MoveDto, void(*)(MoveDto*)>;

                    auto* v = std::get_if<MoveDtoPtr>(&e->payload);
                    if (v == nullptr) break;

                    MoveDto* dto = v->get(); // 생포인터 추출


                    auto inputVector = dto->InputVector;
                    auto pitch = dto->inputPitch;
                    auto yaw = dto->inputYaw;

                    auto player = SessionUtil::GetPlayerFromPeer(e->peer);
                    if (player != nullptr && !player->playerComponent.isNull()) {
                        player->playerComponent->Move(inputVector, pitch, yaw);
                    }
                    break;
                }case SocketEventType::Progress: {
                    using ProgressDtoPtr = std::unique_ptr<LoadingProgressDto, void(*)(LoadingProgressDto*)>;
                    auto* v = std::get_if<ProgressDtoPtr>(&e->payload);
                    if (v == nullptr) break;

                    LoadingProgressDto* dto = v->get();

                    auto player = SessionUtil::GetPlayerFromPeer(e->peer);
                    if (player != nullptr) {
                        uint8_t safeProgress = std::min<uint8_t>(dto->progress, 100);
                        player->status.loadingProgress = safeProgress;

                        // 2. DTO 풀에서 꺼내기
                        ProgressNotifyDto* rawNotifyDto = ObjectPool<ProgressNotifyDto>::GetInstance().Acquire();
                        rawNotifyDto->publicId = player->publicKey;
                        rawNotifyDto->progress = safeProgress; // dto->progress 대신 clamp된 값 사용

                        auto notifyDto = std::unique_ptr<ProgressNotifyDto, void(*)(ProgressNotifyDto*)>(
                            rawNotifyDto,
                            static_cast<void(*)(ProgressNotifyDto*)>([](ProgressNotifyDto* p) {
                                ObjectPool<ProgressNotifyDto>::GetInstance().Release(p);
                            })
                        );

                        BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
                        rawEvent->type = SocketEventType::ProgressNotify;
                        rawEvent->payload = std::move(notifyDto);
                        rawEvent->target.clear(); // 이전 데이터 찌꺼기 초기화
                        std::shared_ptr<BroadCastEvent> event(
                            rawEvent,
                            [](BroadCastEvent* p) {
                                p->payload = nullptr; // variant 내부의 DTO 포인터를 명시적으로 날려서 DTO 풀 반납 트리거
                                ObjectPool<BroadCastEvent>::GetInstance().Release(p); // Event 자체를 풀에 반납
                            }
                        );

                        this->BroadcastEvent(event);
                    }
                    break;
                }
                case SocketEventType::Interact: {
                    using InteractDtoPtr = std::unique_ptr<InteractDto, void(*)(InteractDto*)>;
                    auto* v = std::get_if<InteractDtoPtr>(&e->payload);
                    if (v == nullptr) break;

                    auto player = SessionUtil::GetPlayerFromPeer(e->peer);
                    if (player == nullptr || player->playerComponent.isNull()) break;

                    // 클라 미러: 눈위치(cam.localPosition=0,1.74,0) + 조준방향으로 5f 레이캐스트
                    PlayerMoveSnapshot snap = player->playerComponent->GetMoveSnapshot();
                    Vector3 eye = snap.position + player->playerComponent->aimOrigin;
                    float pitchRad = snap.rotation.x * (3.14159265f / 180.0f);
                    float yawRad   = snap.rotation.y * (3.14159265f / 180.0f);
                    Vector3 dir(std::sin(yawRad) * std::cos(pitchRad),
                                -std::sin(pitchRad),
                                std::cos(yawRad) * std::cos(pitchRad));

                    GameObject self = player->playerComponent->gameObject;
                    RaycastHit hit;
                    if (!physicsSystem->Raycast(Ray(eye, dir), 5.0f, LayerMask(0xFFFFFFFF), hit,
                                                [&self](Collider* c) { return !(c->gameObject == self); })) break;
                    if (hit.collider == nullptr || !hit.collider->gameObject) break;

                    auto interactable = hit.collider->gameObject->GetComponent<Interactable>();
                    if (interactable.isNull()) break;
                    interactable->Interact(player);
                    break;
                }
                case SocketEventType::DropWeapon: {
                    using DropDtoPtr = std::unique_ptr<DropWeaponDto, void(*)(DropWeaponDto*)>;
                    auto* v = std::get_if<DropDtoPtr>(&e->payload);
                    if (v == nullptr) break;

                    auto player = SessionUtil::GetPlayerFromPeer(e->peer);
                    if (player == nullptr || player->playerComponent.isNull()) break;
                    auto inventory = player->playerComponent->gameObject->GetComponent<WeaponInventory>();
                    if (inventory.isNull()) break;

                    ComponentHandle<Weapon> dropped = inventory->DropHolding();
                    Weapon* w = dropped.operator->();
                    if (w == nullptr) break;   // 빈손 → 무응답

                    PlayerMoveSnapshot snap = player->playerComponent->GetMoveSnapshot();
                    const WeaponInfo* wInfo = w->GetInfo();
                    Vector3 hp = (wInfo != nullptr) ? wInfo->handlePosition : Vector3::Zero();
                    float pitchRad = snap.rotation.x * (3.14159265f / 180.0f);
                    float yawRad   = snap.rotation.y * (3.14159265f / 180.0f);
                    float cp = std::cos(pitchRad), sp = std::sin(pitchRad);
                    float cy = std::cos(yawRad),   sy = std::sin(yawRad);
                    Vector3 pitched(hp.x, hp.y * cp - hp.z * sp, hp.y * sp + hp.z * cp);   // Rx(pitch)
                    Vector3 off(pitched.x * cy + pitched.z * sy,                            // Ry(yaw)
                                pitched.y,
                                -pitched.x * sy + pitched.z * cy);
                    Vector3 dropPos = snap.position + player->playerComponent->aimOrigin + off;
                    Vector3 fwd(sy, 0.0f, cy);
                    w->DropToWorld(dropPos, fwd, snap.rotation, player->publicKey,
                                   static_cast<uint8_t>(inventory->holdingSlot < 0 ? 0xFF : inventory->holdingSlot));

                    break;
                }
                case SocketEventType::SwapWeapon: {
                    using SwapDtoPtr = std::unique_ptr<SwapWeaponDto, void(*)(SwapWeaponDto*)>;
                    auto* v = std::get_if<SwapDtoPtr>(&e->payload);
                    if (v == nullptr) break;

                    auto player = SessionUtil::GetPlayerFromPeer(e->peer);
                    if (player == nullptr || player->playerComponent.isNull()) break;
                    auto inventory = player->playerComponent->gameObject->GetComponent<WeaponInventory>();
                    if (inventory.isNull()) break;

                    if (!inventory->SwapDir((*v)->dir != 0)) break;   // 그 방향 무기 없음 → 무응답

                    SwapWeaponNotifyDto* raw = ObjectPool<SwapWeaponNotifyDto>::GetInstance().Acquire();
                    raw->playerKey   = player->publicKey;
                    raw->holdingSlot = static_cast<uint8_t>(inventory->holdingSlot);
                    auto dto = std::unique_ptr<SwapWeaponNotifyDto, void(*)(SwapWeaponNotifyDto*)>(
                        raw, [](SwapWeaponNotifyDto* p) { ObjectPool<SwapWeaponNotifyDto>::GetInstance().Release(p); });

                    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
                    rawEvent->type = SocketEventType::SwapWeaponNotify;
                    rawEvent->payload = std::move(dto);
                    rawEvent->target.clear();
                    std::shared_ptr<BroadCastEvent> event(rawEvent, [](BroadCastEvent* p) {
                        p->payload = nullptr;
                        ObjectPool<BroadCastEvent>::GetInstance().Release(p);
                    });
                    this->BroadcastEvent(event);
                    break;
                }
                case SocketEventType::Reload: {
                    using ReloadDtoPtr = std::unique_ptr<ReloadDto, void(*)(ReloadDto*)>;
                    auto* v = std::get_if<ReloadDtoPtr>(&e->payload);
                    if (v == nullptr) break;

                    auto player = SessionUtil::GetPlayerFromPeer(e->peer);
                    if (player == nullptr || player->playerComponent.isNull()) break;
                    auto inventory = player->playerComponent->gameObject->GetComponent<WeaponInventory>();
                    if (inventory.isNull()) break;

                    if (!inventory->Reload()) break;   // 빈손/무한탄약/풀탄창 → 무응답
                    Weapon* held = inventory->GetHolding().operator->();
                    if (held == nullptr) break;

                    ReloadNotifyDto* raw = ObjectPool<ReloadNotifyDto>::GetInstance().Acquire();
                    raw->playerKey   = player->publicKey;
                    raw->slot        = static_cast<uint8_t>(inventory->holdingSlot);
                    raw->currentAmmo = static_cast<uint16_t>(held->currentAmmo);
                    auto dto = std::unique_ptr<ReloadNotifyDto, void(*)(ReloadNotifyDto*)>(
                        raw, [](ReloadNotifyDto* p) { ObjectPool<ReloadNotifyDto>::GetInstance().Release(p); });

                    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
                    rawEvent->type = SocketEventType::ReloadNotify;
                    rawEvent->payload = std::move(dto);
                    rawEvent->target.clear();
                    std::shared_ptr<BroadCastEvent> event(rawEvent, [](BroadCastEvent* p) {
                        p->payload = nullptr;
                        ObjectPool<BroadCastEvent>::GetInstance().Release(p);
                    });
                    this->BroadcastEvent(event);
                    break;
                }

                case SocketEventType::Jump: {
                    using JumpDtoPtr = std::unique_ptr<JumpDto, void(*)(JumpDto*)>;
                    auto* v = std::get_if<JumpDtoPtr>(&e->payload);
                    if (v == nullptr) break;

                    auto player = SessionUtil::GetPlayerFromPeer(e->peer);
                    if (player == nullptr || player->playerComponent.isNull()) break;

                    player->playerComponent->Jump();
                    break;
                }

                case SocketEventType::Shot: {
                    using ShotDtoPtr = std::unique_ptr<ShotDto, void(*)(ShotDto*)>;
                    auto* v = std::get_if<ShotDtoPtr>(&e->payload);
                    if (v == nullptr) break;

                    auto player = SessionUtil::GetPlayerFromPeer(e->peer);
                    if (player == nullptr || player->playerComponent.isNull()) break;
                    auto inventory = player->playerComponent->gameObject->GetComponent<WeaponInventory>();
                    if (inventory.isNull()) break;

                    Weapon* w = inventory->GetHolding().operator->();
                    if (w == nullptr) break;
                    if (!w->TryShoot()) break;

                    PlayerMoveSnapshot snap = player->playerComponent->GetMoveSnapshot();
                    Vector3 eye = snap.position + player->playerComponent->aimOrigin;
                    float pitchRad = snap.rotation.x * (3.14159265f / 180.0f);
                    float yawRad   = snap.rotation.y * (3.14159265f / 180.0f);
                    Vector3 dir(std::sin(yawRad) * std::cos(pitchRad),
                                -std::sin(pitchRad),
                                std::cos(yawRad) * std::cos(pitchRad));

                    ShotNotifyDto* raw = ObjectPool<ShotNotifyDto>::GetInstance().Acquire();
                    raw->playerKey = player->publicKey;
                    raw->origin    = eye;
                    raw->dir       = dir;
                    auto dto = std::unique_ptr<ShotNotifyDto, void(*)(ShotNotifyDto*)>(
                        raw, [](ShotNotifyDto* p) { ObjectPool<ShotNotifyDto>::GetInstance().Release(p); });

                    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
                    rawEvent->type = SocketEventType::ShotNotify;
                    rawEvent->payload = std::move(dto);
                    rawEvent->target.clear();
                    std::shared_ptr<BroadCastEvent> event(rawEvent, [](BroadCastEvent* p) {
                        p->payload = nullptr;
                        ObjectPool<BroadCastEvent>::GetInstance().Release(p);
                    });
                    this->BroadcastEvent(event);
                    break;
                }
                case SocketEventType::HitThis: {
                    constexpr float maxDistance = 1.0f;

                    ComponentHandle<GamePlayManager> gpmCheck = componentManager->FindFirstComponent<GamePlayManager>();
                    if (gpmCheck.isNull() || gpmCheck->currentPhase != InGamePhase::Fighting) break;

                    using HitThisDtoPtr = std::unique_ptr<HitThisDto, void(*)(HitThisDto*)>;
                    auto* v = std::get_if<HitThisDtoPtr>(&e->payload);
                    if (v == nullptr) break;
                    HitThisDto* dto = v->get();

                    auto shooter = SessionUtil::GetPlayerFromPeer(e->peer);
                    if (shooter == nullptr || shooter->playerComponent.isNull()) break;

                    auto targetIt = players->find(dto->targetPublicKey);
                    if (targetIt == players->end()) break;
                    Player& target = targetIt->second;
                    if (target.playerComponent.isNull()) break;


                    Vector3 shooterEye = shooter->playerComponent->gameObject->transform.GetPosition() + shooter->playerComponent->aimOrigin;
                    if (shooterEye.Distance(dto->origin) > maxDistance) break;

                    auto inventory = shooter->playerComponent->gameObject->GetComponent<WeaponInventory>();
                    if (inventory.isNull()) break;
                    Weapon* w = inventory->GetHolding().operator->();
                    if (w == nullptr) break;
                    if (!w->TryShoot()) break;

                    ShotNotifyDto* raw = ObjectPool<ShotNotifyDto>::GetInstance().Acquire();
                    raw->playerKey = shooter->publicKey;
                    raw->origin    = dto->origin;
                    raw->dir       = dto->dir;
                    auto shotDto = std::unique_ptr<ShotNotifyDto, void(*)(ShotNotifyDto*)>(
                        raw, [](ShotNotifyDto* p) { ObjectPool<ShotNotifyDto>::GetInstance().Release(p); });

                    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
                    rawEvent->type = SocketEventType::ShotNotify;
                    rawEvent->payload = std::move(shotDto);
                    rawEvent->target.clear();
                    std::shared_ptr<BroadCastEvent> event(rawEvent, [](BroadCastEvent* p) {
                        p->payload = nullptr;
                        ObjectPool<BroadCastEvent>::GetInstance().Release(p);
                    });

                    IHitValidator(dto, shooter, &target, w);
                    break;
                }
                case SocketEventType::HitStructure: {
                    // 플레이어가 아닌 오브젝트(벽 등) 명중 클레임 — 랙보상 리와인드 불필요(구조물은 안 움직이므로), 레이어로만 필터링해 재판정
                    ComponentHandle<GamePlayManager> gpmCheck = componentManager->FindFirstComponent<GamePlayManager>();
                    if (gpmCheck.isNull() || gpmCheck->currentPhase != InGamePhase::Fighting) break;

                    using HitStructureDtoPtr = std::unique_ptr<HitStructureDto, void(*)(HitStructureDto*)>;
                    auto* v = std::get_if<HitStructureDtoPtr>(&e->payload);
                    if (v == nullptr) break;
                    HitStructureDto* dto = v->get();

                    auto shooter = SessionUtil::GetPlayerFromPeer(e->peer);
                    if (shooter == nullptr || shooter->playerComponent.isNull()) break;

                    constexpr float maxDistance = 1.0f;
                    Vector3 shooterEye = shooter->playerComponent->gameObject->transform.GetPosition() + shooter->playerComponent->aimOrigin;
                    if (shooterEye.Distance(dto->origin) > maxDistance) break;

                    auto inventory = shooter->playerComponent->gameObject->GetComponent<WeaponInventory>();
                    if (inventory.isNull()) break;
                    Weapon* w = inventory->GetHolding().operator->();
                    if (w == nullptr) break;
                    if (!w->TryShoot()) break;

                    ShotNotifyDto* raw = ObjectPool<ShotNotifyDto>::GetInstance().Acquire();
                    raw->playerKey = shooter->publicKey;
                    raw->origin    = dto->origin;
                    raw->dir       = dto->dir;
                    auto shotDto = std::unique_ptr<ShotNotifyDto, void(*)(ShotNotifyDto*)>(
                        raw, [](ShotNotifyDto* p) { ObjectPool<ShotNotifyDto>::GetInstance().Release(p); });

                    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
                    rawEvent->type = SocketEventType::ShotNotify;
                    rawEvent->payload = std::move(shotDto);
                    rawEvent->target.clear();
                    std::shared_ptr<BroadCastEvent> event(rawEvent, [](BroadCastEvent* p) {
                        p->payload = nullptr;
                        ObjectPool<BroadCastEvent>::GetInstance().Release(p);
                    });
                    this->BroadcastEvent(event);

                    // Ground 레이어(구조물이 배치되는 레이어)로만 제한 — 플레이어(Default 레이어)는 애초에 후보에 안 들어옴
                    RaycastHit hit;
                    Ray ray(dto->origin, dto->dir);
                    bool hasResult = physicsSystem->Raycast(ray, 300, _groundMask, hit, nullptr);
                    if (!hasResult) break;
                    if (hit.collider->gameObject.GetId() != dto->targetObjectId) break;   // 클라 주장과 서버 판정 불일치 → 무시(안티치트)

                    ApplyKnockback(hit.collider, hit.point, dto->dir, w);
                    break;
                }
                case SocketEventType::Ping: {
                    //pong
                    break;
                }
                case SocketEventType::Input: break;
                case SocketEventType::Setup: break;
                case SocketEventType::Update: break;
                default: break;
            }
        } catch (const std::exception& ex) {
            std::cout << "[Event Process Error] " << ex.what() << std::endl;
        }
    }
}
void GameSession::Tick() {
    tick++;

    auto sectionStart = std::chrono::steady_clock::now();
    auto markSection = [&sectionStart](std::atomic<long long>& target) {
        auto now = std::chrono::steady_clock::now();
        target.store(std::chrono::duration_cast<std::chrono::microseconds>(now - sectionStart).count(), std::memory_order_relaxed);
        sectionStart = now;
    };

    ProcessEventQueue();
    markSection(lastEventQueueMicros);

    UpdateComponents();
    markSection(lastUpdateComponentsMicros);

    FlushGameObject();
    markSection(lastFlushGameObjectMicros);

    BroadcastMovements();
    markSection(lastBroadcastMovementsMicros);

    BroadcastObjectMovements();
    markSection(lastBroadcastObjectMovementsMicros);

    CheckAllPlayerDisconnected();
    markSection(lastCheckDisconnectedMicros);
#ifdef _WIN64
    UpdateRenderBuffer();
#endif
}
void GameSession::UpdateComponents() const {
    componentManager->UpdateComponents();
    //Log("컴포넌트업데이트성공했어요");
}
void GameSession::FlushGameObject() const {
    objectManager->Flush();
}
void GameSession::SetCharacter(const CharacterSetDto& dto) const {
    for (auto v : dto.elements) {
        for (auto& p : *players | std::views::values) {
            if (p.userId == v.userId) {
                p.SetCharacter(static_cast<uint8_t>(std::stoi(v.characterId)));
                break;
            }
        }
    }
}

struct HitRewinder {
    GameObject targetObject;
    Vector3 originalPosition;
    bool restored = false;

    HitRewinder(GameSession* session, Player* shooter, Player* rewindTarget) {

        if (rewindTarget == nullptr || rewindTarget->playerComponent.isNull()) return;
        PlayerComponent* pc = rewindTarget->playerComponent.operator->();
        if (pc->historySize <= 0) return;
        if (pc->validHistorySamples <= 0) return;   // 유효기록 자체가 없으면 리와인드 자체를 포기(=현재위치로 검증)
        int maxSteps = (std::min)(pc->historySize, pc->validHistorySamples);
        // 랙보상은 "쏜 사람이 쏘던 순간 자기 화면에서 본 위치"로 되감는 것 — shooter의 핑 기준이어야 함(맞는 사람 핑 아님)
        float rewindSeconds = SessionUtil::GetRewindOffsetSeconds(shooter);
        auto targetTime = std::chrono::steady_clock::now()
            - std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<float>(rewindSeconds));

        int idx = pc->lastHistoryIndex;
        int steps = 0;
        while (steps < maxSteps && pc->history[idx].timestamp > targetTime) {
            idx = (idx - 1 + pc->historySize) % pc->historySize;
            ++steps;
        }
        // targetTime보다 오래된(같거나 이전) 첫 슬롯에서 멈춤. maxSteps를 다 돌아도 못 찾으면(=버퍼 안에 그만큼 오래된 기록이 없음) 유효 기록 중 가장 오래된 값으로 자연스럽게 폴백됨.

        targetObject = pc->gameObject;
        originalPosition = targetObject->transform.GetPosition();
        targetObject->transform.SetPosition(pc->history[idx].position);
        restored = true;
    }


    ~HitRewinder() {
        if (restored) targetObject->transform.SetPosition(originalPosition);
    }
};


///dto 기반으로 맞았는지 안 맞았는지 rewind해서 확인하고 맞으면 맞았다고 값 처리 및 전파까지 하는 함수
void GameSession::IHitValidator(HitThisDto* hitThisDto, Player* shooter, Player* target, Weapon* weapon) {
    LayerMask layer_mask = LayerMask(_groundMask | _playerMask);
    RaycastHit hit;
    Ray ray = Ray(hitThisDto->origin, hitThisDto->dir);
    bool hasResult;
    {
        HitRewinder rewinder(this, shooter, target);
        hasResult = physicsSystem->Raycast(ray, 300, layer_mask, hit, [=](Collider* c){
                if (c->gameObject->tag != TagManager::GetObjectTagFromString("Player")) return true;
                return c->gameObject == target->playerComponent->gameObject;
            }
        );
    }
    if (hasResult == false) return;

    if (hit.collider->gameObject == target->playerComponent->gameObject) {
        auto dmg = target->playerComponent->CalcDamage(hit.collider, weapon);
        bool died = target->playerComponent->TakeDamage(dmg);
        uint8_t hitPart = (hit.collider->GetShapeType() == ColliderType::Sphere) ? 1 : 0;

        HitDto* raw = ObjectPool<HitDto>::GetInstance().Acquire();
        raw->victimKey    = target->publicKey;
        raw->attackerKey  = shooter->publicKey;
        raw->hitPart      = hitPart;
        raw->remainingHp  = static_cast<uint16_t>(target->playerComponent->GetCurrentHp());
        raw->hitPosition  = hit.point;
        auto hitDto = std::unique_ptr<HitDto, void(*)(HitDto*)>(
            raw, [](HitDto* p) { ObjectPool<HitDto>::GetInstance().Release(p); });

        BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
        rawEvent->type = SocketEventType::HitNotify;
        rawEvent->payload = std::move(hitDto);
        rawEvent->target.clear();
        std::shared_ptr<BroadCastEvent> event(rawEvent, [](BroadCastEvent* p) {
            p->payload = nullptr;
            ObjectPool<BroadCastEvent>::GetInstance().Release(p);
        });
        this->BroadcastEvent(event);

        if (died) {
            target->playerComponent->Death();
            shooter->status.kill++;
            target->status.death++;

            DeathDto* deathRaw = ObjectPool<DeathDto>::GetInstance().Acquire();
            deathRaw->victimKey = target->publicKey;
            deathRaw->killerKey = shooter->publicKey;
            auto deathDto = std::unique_ptr<DeathDto, void(*)(DeathDto*)>(
                deathRaw, [](DeathDto* p) { ObjectPool<DeathDto>::GetInstance().Release(p); });

            BroadCastEvent* deathEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
            deathEvent->type = SocketEventType::Death;
            deathEvent->payload = std::move(deathDto);
            deathEvent->target.clear();
            std::shared_ptr<BroadCastEvent> devent(deathEvent, [](BroadCastEvent* p) {
                p->payload = nullptr;
                ObjectPool<BroadCastEvent>::GetInstance().Release(p);
            });
            this->BroadcastEvent(devent);

            ComponentHandle<GamePlayManager> gpm = componentManager->FindFirstComponent<GamePlayManager>();
            if (!gpm.isNull()) gpm->CheckOneTeamRemain();
        }
    }
    // 플레이어(타겟)를 조준했지만 실제로는 중간의 구조물(벽 등)에 막힌 경우 — 위 if에서 플레이어는 이미 다 처리되고 여기로 안 옴,
    // 그래도 재구조화 대비 방어적으로 한 번 더 태그 체크(플레이어 리지드바디는 절대 안 건드림)
    else if (hit.collider->gameObject->tag != TagManager::GetObjectTagFromString("Player")) {
        ApplyKnockback(hit.collider, hit.point, hitThisDto->dir, weapon);
    }
}

void GameSession::ApplyKnockback(Collider* hitCollider, const Vector3& hitPoint, const Vector3& dir, Weapon* weapon) {
    ComponentHandle<Rigidbody> rb = hitCollider->gameObject->GetComponent<Rigidbody>();
    if (rb.isNull()) return;

    constexpr float KNOCKBACK_FORCE_MULTIPLIER = 0.5f;
    const WeaponInfo* info = weapon->GetInfo();
    float force = info ? info->bodyDamage * KNOCKBACK_FORCE_MULTIPLIER : 50.0f;
    rb->AddImpulseAtPoint(hitPoint, dir * force);
}

void GameSession::CheckAllPlayerDisconnected() {
    bool anyConnected = false, anyDisconnected = false;
    for (auto& p : *players | std::views::values) {
        if (p.status.networkStatus == connected)    anyConnected = true;
        if (p.status.networkStatus == disconnected) anyDisconnected = true;
    }
    if (anyConnected || !anyDisconnected) abandonedTicks = 0;   // 로딩 대기(not_connected만)는 카운트 안 함
    else if (++abandonedTicks > 30 * 30) {                      // 30tps × 30초
        LOG_INFO("session abandoned, self-stopping: " + sessionId);
        running = false;   // 자기 스레드라 join 불가 → 루프 자연 종료, 수확은 리퍼가
    }
}
constexpr std::chrono::nanoseconds TIME_STEP(33333334);

void GameSession::TryTick() {
    if (!(isRunning.load() and running)) return;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - previousTickTime;
    previousTickTime = now;
    if (elapsed > std::chrono::milliseconds(250)) {
        elapsed = std::chrono::milliseconds(250);
    }
    lag += elapsed;
    while (lag >= TIME_STEP) {
        time.DeltaTime = std::chrono::duration<float>(TIME_STEP).count();

        auto tickStart = std::chrono::steady_clock::now();
        Tick();
        auto tickDurationUs = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - tickStart).count();
        lastTickMicros.store(tickDurationUs, std::memory_order_relaxed);
        tpsTickCounter.fetch_add(1, std::memory_order_relaxed);

        lag -= TIME_STEP;
    }
    lagMillis.store(std::chrono::duration_cast<std::chrono::milliseconds>(lag).count(), std::memory_order_relaxed);
    if (now - tpsWindowStart >= std::chrono::seconds(1)) {
        currentTps.store(tpsTickCounter.exchange(0, std::memory_order_relaxed), std::memory_order_relaxed);
        tpsWindowStart = now;
    }
}

void GameSession::Start() {
    LOG_INFO("new session is running");

    while (isRunning.load() and running) {
        TryTick();

        auto remainingTime = TIME_STEP - lag;
        if (remainingTime > std::chrono::milliseconds(2)) {
            std::this_thread::sleep_for(remainingTime - std::chrono::milliseconds(1));
        } else {
            std::this_thread::yield();
        }
    }
}
void GameSession::Stop() {
    LOG_INFO("session stopped id: " + initInfo.gameId);
    running = false;
    if (gameThread.joinable()) {
        gameThread.join();
    }
    isStopped = true;
}

double GameSession::GetThreadCpuPercent() {
#ifdef _WIN64
    if (!gameThread.joinable()) return -1.0;   // 이미 join된(종료된) 스레드는 native_handle 무효

    FILETIME createTime, exitTime, kernelTime, userTime;
    if (!GetThreadTimes(gameThread.native_handle(), &createTime, &exitTime, &kernelTime, &userTime)) return -1.0;

    ULARGE_INTEGER k{}, u{}, c{};
    k.LowPart = kernelTime.dwLowDateTime; k.HighPart = kernelTime.dwHighDateTime;
    u.LowPart = userTime.dwLowDateTime;   u.HighPart = userTime.dwHighDateTime;
    c.LowPart = createTime.dwLowDateTime; c.HighPart = createTime.dwHighDateTime;

    FILETIME nowFt;
    GetSystemTimeAsFileTime(&nowFt);
    ULARGE_INTEGER nowU{};
    nowU.LowPart = nowFt.dwLowDateTime; nowU.HighPart = nowFt.dwHighDateTime;

    unsigned long long cpuTimeNow  = k.QuadPart + u.QuadPart;   // 스레드 생성 이후 누적 CPU(커널+유저) 시간, 100ns 단위
    unsigned long long wallTimeNow = nowU.QuadPart - c.QuadPart; // 스레드 생성 이후 누적 벽시계 시간, 100ns 단위

    double percent;
    if (!_hasCpuSnapshot) {
        // 최초 호출: 스냅샷이 없으니 누적 평균으로 대체
        percent = (wallTimeNow == 0) ? 0.0 : static_cast<double>(cpuTimeNow) / static_cast<double>(wallTimeNow) * 100.0;
    } else {
        // 직전 호출 이후 구간만 델타로 계산 - 작업관리자의 "최근 사용률"과 동일한 개념
        unsigned long long cpuDelta  = cpuTimeNow  - _prevCpuTime100ns;
        unsigned long long wallDelta = wallTimeNow - _prevWallTime100ns;
        percent = (wallDelta == 0) ? 0.0 : static_cast<double>(cpuDelta) / static_cast<double>(wallDelta) * 100.0;
    }

    _prevCpuTime100ns = cpuTimeNow;
    _prevWallTime100ns = wallTimeNow;
    _hasCpuSnapshot = true;

    return percent;
#else
    return -1.0;
#endif
}

void GameSession::Init(const std::string& sessionId, const GameSetupBoddari& initInfo) {
    this->players = std::make_shared<std::map<uint64_t, Player>>();
    this->sessionId = std::move(sessionId);
    this->initInfo = initInfo;
    uint8_t publicKey=0;
    auto res = physicsSystem->Init(MapInfo(initInfo.mapId), this) ;
    if (!res){/*todo: 매칭 취소 로직*/ return; }
    std::cout<<"게임 ID "<<sessionId<<"에서 맵 생성중. 맵 아이디:"<<initInfo.mapId<< std::endl;
    //std::cout<<"New Session Enqueue Players:"<< std::endl;
    for (auto p : initInfo.players)
    {
        //std::cout<<p.id<<std::endl;

        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<uint64_t> dist(1, (std::numeric_limits<uint64_t>::max)());

        playerStatus newStatus = playerStatus();
        newStatus.team = p.team;
        newStatus.characterId = static_cast<uint8_t>(std::stoi(p.characterId));
        Player newPlayer = Player(p.id, p.name,p.key,publicKey, newStatus);
        (*players)[publicKey] = newPlayer;
        publicKey++;
        //std::cout<<"Enqueue Succeced"<<std::endl;
    }

    _playerMask = physicsSystem->layerManager.GetMask("Default");
    _groundMask = physicsSystem->layerManager.GetMask("Ground");
}

bool GameSession::reset() {
    return false;
}

void GameSession::cleanUp() {
    Stop();
}

#ifdef _WIN64
#include "./FhishiX/Renderer.h"

int GameSession::InsertRenderer(const Renderer &renderer) {
    //std::cout <<"OWNER: "<<renderer.owner->gameSession<<" and handle:  "<<renderer.owner.handleSession<< "InsertRenderer Started: " << renderer.owner->name << std::endl;
    if (!usableRenderersIndex.empty()) {
        std::cout<<"RenderIndex resycle"<<std::endl;
        int idx = usableRenderersIndex.front();
        usableRenderersIndex.pop();
        renderers[idx] = renderer;
        return idx;
    }

    renderers.push_back(renderer);

    return renderers.size() - 1;
}




void GameSession::DeleteRenderer(int index) {
    renderers[index].isAlive = false;
    usableRenderersIndex.push(index);
}

void GameSession::UpdateRenderBuffer() {
    std::vector<RenderPacket>& writeBuffer = buffers[writeIdx];
    writeBuffer.clear();


    for (const auto& renderer : renderers) {
        if (!renderer.enable || !renderer.isAlive || renderer.owner == GameObject::NullPTR() || renderer.owner.targetId == -1){
            std::cout << "불량 renderer 발생"<<std::endl;
            continue;
        };
        GameObjectArgument* rawObj = renderer.owner.operator->();
        if (rawObj == nullptr) continue;
        Transform* safeTransform = &rawObj->transform;
        RenderPacket packet;
        packet.mesh = renderer.mesh;
        packet.color = renderer.color;
        packet.isWireframe = renderer.isWireframe;

        if (safeTransform) {
            Matrix4 myMat = safeTransform->GetWorldMatrix().Transpose();
            DirectX::XMMATRIX baseTransform = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(myMat.m.data()));
            DirectX::XMMATRIX mat = DirectX::XMMatrixScaling(renderer.localScale.x, renderer.localScale.y, renderer.localScale.z)
            * DirectX::XMMatrixTranslation(renderer.localOffset.x, renderer.localOffset.y, renderer.localOffset.z)
            * baseTransform;
            DirectX::XMStoreFloat4x4(&packet.worldMatrix, mat);
        }

        writeBuffer.push_back(packet);
    }

    {
        std::lock_guard<std::mutex> lock(renderBufferMutex);
        std::swap(writeIdx, nextIdx);
        isRenderDataReady = true;
    }
}

const std::vector<RenderPacket> *GameSession::GetRenderPackets() {
    {
        if (isRenderDataReady.load()) { // 새 데이터가 있을 때만 스왑!
            std::swap(readIdx, nextIdx);
            isRenderDataReady = false;
        }
    }
    return &buffers[readIdx];
}
#endif

std::shared_ptr<Player> GameSession::RegistUser(const std::string &userKey, ENetPeer *peer) const {
    if (!isRunning.load() and running) return nullptr;
    for(auto& v : *this->players | std::views::values)
    {
        if (v.assignKey == userKey) {
            v.peer = peer;
            v.peerConnectId = peer->connectID;
            peer->data = &v;

            return std::make_shared<Player>(v);
        }
    }
    return nullptr;
}

void GameSession::ProcessEvent(GameEventPtr event)
{
    eventQueue.enqueue(std::move(event));
}


void GameSession::BroadcastEvent(const std::shared_ptr<BroadCastEvent>& event) {
    if (!players || players->empty()) return;

    enet_uint32 packetFlags = GetPacketFlags(event->type);

    std::vector<uint8_t> buffer;

    std::visit([&buffer](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (!std::is_same_v<T, std::nullptr_t>) {
            if (arg) {
                buffer.resize(arg->GetDtoBinaryLength());
                arg->ToBinary(buffer.data());
            }
        }
    }, event->payload);

    if (buffer.empty()) return;

    bool broadcastToAll = event->target.empty();

    for (const auto &player: *players | std::views::values) {
        if (player.peer != nullptr) { // 연결 상태 체크는 ENet 스레드에서 최종 확인
            if (broadcastToAll || std::find(event->target.begin(), event->target.end(), player.peer) != event->target.end()) {
                EnetClient::GetInstance()->EnqueueSend(player.peer, player.peerConnectId, buffer, packetFlags);
            }
        }
    }
}

void GameSession::BroadcastMovements() {
    if (!players || players->empty()) return;

    BroadcastPlayerMoveDto* rawDto = ObjectPool<BroadcastPlayerMoveDto>::GetInstance().Acquire();
    rawDto->players.clear();

    auto* pool = componentManager->GetOrCreatePool<PlayerComponent>();
    for (auto& pc : *pool) {
        if (!pc.isActive || pc.willDead) continue;
        auto snap = pc.GetMoveSnapshot();

        PlayerMoveEntry entry;
        entry.publicKey = pc.publicKey;
        entry.position = snap.position;
        entry.rotation = snap.rotation;
        entry.velocity = snap.velocity;
        rawDto->players.push_back(entry);
    }

    if (rawDto->players.empty()) {
        ObjectPool<BroadcastPlayerMoveDto>::GetInstance().Release(rawDto);
        return;
    }

    auto moveDto = std::unique_ptr<BroadcastPlayerMoveDto, void(*)(BroadcastPlayerMoveDto*)>(
        rawDto,
        static_cast<void(*)(BroadcastPlayerMoveDto*)>([](BroadcastPlayerMoveDto* p) {
            ObjectPool<BroadcastPlayerMoveDto>::GetInstance().Release(p);
        })
    );

    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
    rawEvent->type = SocketEventType::PlayerMove;
    rawEvent->payload = std::move(moveDto);
    rawEvent->target.clear(); // 비움 = 전체 방송(본인 포함, 권위 좌표라 reconciliation용)

    std::shared_ptr<BroadCastEvent> event(
        rawEvent,
        [](BroadCastEvent* p) {
            p->payload = nullptr; // variant 딜리터 트리거 → DTO 풀 반납
            ObjectPool<BroadCastEvent>::GetInstance().Release(p);
        }
    );

    this->BroadcastEvent(event);
}

void GameSession::BroadcastObjectMovements() {
    if (!players || players->empty()) return;

    constexpr uint8_t OBJECT_RESEND_TICKS = 5;

    BroadcastMoveDto* rawDto = ObjectPool<BroadcastMoveDto>::GetInstance().Acquire();
    rawDto->objects.clear();

    auto* pool = componentManager->GetOrCreatePool<SynchronizedObject>();
    for (auto& so : *pool) {
        if (!so.isActive || !so.gameObject) continue;

        const Vector3& pos = so.gameObject->transform.GetPosition();
        Vector3 rot = so.gameObject->transform.GetRotation().ToEuler();

        if (pos != so.lastSentPos || rot != so.lastSentRot) so.resendTicks = OBJECT_RESEND_TICKS;
        if (so.resendTicks == 0) continue;
        so.resendTicks--;

        so.lastSentPos = pos;
        so.lastSentRot = rot;

        ObjectMoveEntry entry;
        entry.targetId = static_cast<uint32_t>(so.gameObject.targetId);
        entry.position = pos;
        entry.rotation = rot;
        rawDto->objects.push_back(entry);
    }

    if (rawDto->objects.empty()) {
        ObjectPool<BroadcastMoveDto>::GetInstance().Release(rawDto);
        return;
    }

    auto moveDto = std::unique_ptr<BroadcastMoveDto, void(*)(BroadcastMoveDto*)>(
        rawDto,
        static_cast<void(*)(BroadcastMoveDto*)>([](BroadcastMoveDto* p) {
            ObjectPool<BroadcastMoveDto>::GetInstance().Release(p);
        })
    );

    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
    rawEvent->type = SocketEventType::ObjectMove;
    rawEvent->payload = std::move(moveDto);
    rawEvent->target.clear();

    std::shared_ptr<BroadCastEvent> event(
        rawEvent,
        [](BroadCastEvent* p) {
            p->payload = nullptr;
            ObjectPool<BroadCastEvent>::GetInstance().Release(p);
        }
    );

    this->BroadcastEvent(event);
}

GameObject GameSession::SpawnSyncObject(uint32_t prefabId, const Vector3& pos) {
    GameObject obj = PrefabManager::Instantiate(prefabId, this);
    if (!obj) return obj;   // 매핑 없는 prefabId → 방송 없이 종료

    obj->transform.SetPosition(pos);

    auto sync = obj->GetComponent<SynchronizedObject>();
    if (!sync.isNull()) sync->resendTicks = 5;   // 초기 플러시 → 이동 파이프라인 탑승

    GenerateObjectDto* rawDto = ObjectPool<GenerateObjectDto>::GetInstance().Acquire();
    rawDto->targetId = static_cast<uint32_t>(obj.targetId);
    rawDto->prefabId = static_cast<uint8_t>(prefabId);
    rawDto->position = pos;

    auto genDto = std::unique_ptr<GenerateObjectDto, void(*)(GenerateObjectDto*)>(
        rawDto,
        static_cast<void(*)(GenerateObjectDto*)>([](GenerateObjectDto* p) {
            ObjectPool<GenerateObjectDto>::GetInstance().Release(p);
        })
    );

    BroadCastEvent* rawEvent = ObjectPool<BroadCastEvent>::GetInstance().Acquire();
    rawEvent->type = SocketEventType::GenerateObject;
    rawEvent->payload = std::move(genDto);
    rawEvent->target.clear();

    std::shared_ptr<BroadCastEvent> event(
        rawEvent,
        [](BroadCastEvent* p) {
            p->payload = nullptr;
            ObjectPool<BroadCastEvent>::GetInstance().Release(p);
        }
    );

    this->BroadcastEvent(event);
    return obj;
}




