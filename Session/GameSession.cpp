#include "GameSession.h"

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
#include "../Socket/dto/MapInitDto.h"
class CapsuleCollider;

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

//스레드로 실행
void GameSession::RunAsync() {
    running = true;
    gameThread = std::thread([this]() {
        this->Start();
    });
    isStopped = false;
}

void GameSession::ProcessEventQueue() {
    std::unique_lock<std::mutex> lock(queueMutex);
    while (!eventQueue.empty()) {
        if (!this->running or !isRunning.load()) break;

        GameEventPtr e = std::move(eventQueue.front());
        eventQueue.pop();
        lock.unlock(); // 이벤트 처리 동안 unlock

        try {
            switch (e->type) {
                case SocketEventType::Assign: {
                    using AssignDtoPtr = std::unique_ptr<AssignRequestDto, void(*)(AssignRequestDto*)>;
                    auto* v = std::get_if<AssignDtoPtr>(&e->payload);
                    if (v == nullptr) break;

                    AssignRequestDto* dto = v->get();

                    for (auto& val : *players | std::views::values) {
                        if (val.assignKey == dto->Key) {
                            val.peer = e->peer;
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

                    auto secretKey = dto->UserSecretKey;

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

        lock.lock(); // 처리 완료되면 다음 이벤트 처리 위해 다시 lock
    }
}
void GameSession::Tick() {
    tick++;
    ProcessEventQueue();
    UpdateComponents();
    FlushGameObject();
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
                p.SetCharacter(v.characterId);
                break;
            }
        }
    }

}
constexpr std::chrono::nanoseconds TIME_STEP(33333334);
void GameSession::Start() {
    LOG_INFO("session is running on port " + std::to_string(Consts::port));
    Log("으아아악돌아가요");
    // Run server loop

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> distrib(0, 255);
    std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();
    std::chrono::nanoseconds lag(0);
    while (isRunning.load() and running) {
        auto now = std::chrono::steady_clock::now();

        auto elapsed = now - previousTime;
        previousTime = now;

        time.DeltaTime = std::chrono::duration<float>(elapsed).count();

        if (elapsed > std::chrono::milliseconds(250)) {
            elapsed = std::chrono::milliseconds(250);
        }
        lag += elapsed;
        while (lag >= TIME_STEP) {
            time.FixedDeltaTime = std::chrono::duration<float>(TIME_STEP).count();
            Tick();
            lag -= TIME_STEP;
        }
        auto remainingTime = TIME_STEP - lag;
        if (remainingTime > std::chrono::milliseconds(2)) {
            // 시간이 2ms 이상 남았을 때만 Sleep
            std::this_thread::sleep_for(remainingTime - std::chrono::milliseconds(1));
        } else {
            // 시간이 별로 없으면 Sleep 대신 양보만 하여 즉시 반응 준비
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

void GameSession::Init(const std::string& sessionId, const GameSetupBoddari& initInfo) {
    this->players = std::make_shared<std::map<uint64_t, Player>>();
    this->sessionId = std::move(sessionId);
    this->initInfo = initInfo;
    uint64_t privateKey;
    uint8_t publicKey=0;
    auto res = physicsSystem->Init(MapInfo(initInfo.mapId), this) ;
    if (!res){/*todo: 매칭 취소 로직*/ return; }
    std::cout<<"게임 ID "<<sessionId<<"에서 맵 생성중. 맵 아이디:"<<initInfo.mapId<< std::endl;
    std::cout<<"New Session Enqueue Players:"<< std::endl;
    for (auto p : initInfo.players)
    {
        std::cout<<p.id<<std::endl;

        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<uint64_t> dist(1, (std::numeric_limits<uint64_t>::max)());

        do {
            privateKey = dist(rng);
        } while (SessionUtil::ContainsPrivateKey(*players, privateKey));
        playerStatus newStatus = playerStatus();
        Player newPlayer = Player(p.id, p.name,p.key, privateKey, publicKey++, newStatus);
        (*players)[privateKey] = newPlayer;
        std::cout<<"Enqueue Succeced"<<std::endl;
    }
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
    std::cout <<"OWNER: "<<renderer.owner->gameSession<<" and handle:  "<<renderer.owner.handleSession<< "InsertRenderer Started: " << renderer.owner->name << std::endl;
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
            DirectX::XMMATRIX mat = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(renderer.localOffset.x, renderer.localOffset.y, renderer.localOffset.z),baseTransform);
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
            peer->data = &v;

            return std::make_shared<Player>(v);
        }
    }
    return nullptr;
}

void GameSession::ProcessEvent(GameEventPtr event)
{
    std::lock_guard<std::mutex> lock(queueMutex);

    eventQueue.push(std::move(event));

    queueCV.notify_one();
}


void GameSession::BroadcastEvent(const std::shared_ptr<BroadCastEvent>& event) {
    if (!players || players->empty()) return;

    enet_uint32 packetFlags = (event->type == SocketEventType::Move)
                                ? ENET_PACKET_FLAG_UNSEQUENCED
                                : ENET_PACKET_FLAG_RELIABLE;

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

    for (const auto& [privateKey, player] : *players) {
        if (player.peer != nullptr) { // 연결 상태 체크는 ENet 스레드에서 최종 확인
            if (broadcastToAll || std::find(event->target.begin(), event->target.end(), player.peer) != event->target.end()) {
                EnetClient::GetInstance()->EnqueueSend(player.peer, buffer, packetFlags);
            }
        }
    }
}

