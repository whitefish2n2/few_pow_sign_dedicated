#include "GameSession.h"

#include <iostream>
#include <enet/enet.h>
#include <thread>
#include <random>
#include <string>
#include <ranges>
#include <utility>

#include "SessionContext.h"
#include "Game/Player.h"
#include "SessionUtil.h"
#include "../Constants.h"
#include "../ServerStatics.h"
#include "../Socket/dto/AssignDto.h"
#include "../Socket/dto/SocketEventType.h"
#include "../util/util.h"
#include "FhishiX/gameobject/GameObjectManager.h"
#include "Game/MapManager.h"
#include "Game/Map/MapConstructer/PhysicsSystemConstructor.h"
#include "../Socket/BroadcastMoveDto.h"
#include "../util/Log.h"

GameSession::GameSession() {
    objectManager = std::make_unique<GameObjectManager>();
    objectManager->ownerSession = this;
    componentManager = std::make_unique<ComponentManager>();
    componentManager->ownerSession = this;
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
        std::shared_ptr<GameEvent> e;
        e = eventQueue.front();
        eventQueue.pop();
        lock.unlock();//이벤트 처리동안 unlock
        try
        {
            switch (e->type)
            {
                case SocketEventType::Assign: {
                    auto v = (std::get_if<std::shared_ptr<AssignRequestDto>>(&e->payload));
                    if (v==nullptr) break;
                    auto dto = *v;
                    for (const auto& val: *players | std::views::values) {
                        if (val.assignKey == dto->Key){
                            //val.peer = dto->

                        }
                    }
                    //todo
                    break;
                }

                [[likely]]case SocketEventType::Input:
                    break;
                case SocketEventType::Move:
                {
                    auto dto = (std::get_if<std::shared_ptr<MoveDto>>(&e->payload));
                    if (dto==nullptr) break;
                    auto secretKey = (*dto)->UserSecretKey;
                    auto inputVector = (*dto)->InputVector;
                    SessionUtil::GetPlayerFromPeer(e->peer)->Move(inputVector);
                    break;
                }
                case SocketEventType::Setup:
                    break;
                case SocketEventType::Update:
                    break;
                case SocketEventType::Hit:
                    break;
                case SocketEventType::Swap:
                    break;
                case SocketEventType::Generate:
                    break;
                case SocketEventType::Default:
                    break;
                default: ;
            }
        }catch (const std::exception& ex)
        {
            std::cout << ex.what()<<std::endl;
        }
        lock.lock();//처리 완료되면 다음 이벤트 처리 위해 다시 lock
    }
}
void GameSession::Tick() {
    tick++;
    ProcessEventQueue();
    UpdateComponents();
#ifdef _WIN64
    UpdateRenderBuffer();
#endif
}
void GameSession::UpdateComponents() const {
    componentManager->UpdateComponents();
    //Log("컴포넌트업데이트성공했어요");
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

    // 게임 내(스레드 내부) 전역 매니저 인스턴스 초기화, 생성
    gameSessionInstance = this;
    gameObjectManagerInstance = objectManager.get();
    componentManagerInstance = componentManager.get();
    gameObjectManagerInstance->ownerSession  = this;
    componentManagerInstance->ownerSession = this;

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
    auto res = MapManager::GetInstance()->GetPhysicsMapConstructor(MapInfo(initInfo.mapId))->Construct(this) ;

    if (!res){/*todo: 매칭 취소 로직*/ return; }
    std::cout<<"게임 ID "<<sessionId<<"에서 맵 생성중. 맵 아이디:"<<initInfo.mapId<< std::endl;
    std::cout<<"New Session Enqueue Players:"<< std::endl;
    for (auto p : initInfo.players)
    {
        std::cout<<p.id<<std::endl;

        // TODO 이좀 처리해봐
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<uint64_t> dist(1, (std::numeric_limits<uint64_t>::max)());

        do {
            privateKey = dist(rng);
        } while (SessionUtil::ContainsPrivateKey(*players, privateKey));
        player_status newStatus = player_status();
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
    renderers[index].~Renderer();
    usableRenderersIndex.push(index);
}

void GameSession::UpdateRenderBuffer() {
    std::vector<RenderPacket>& writeBuffer = buffers[writeIdx];
    writeBuffer.clear();


    for (const auto& renderer : renderers) {
        if (!renderer.enable || renderer.owner == GameObject::NullPTR() || renderer.owner.targetId == -1){
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
        if (isRenderDataReady.load()) { // ✨ 새 데이터가 있을 때만 스왑!
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

void GameSession::ProcessEvent(std::shared_ptr<GameEvent>& event)
{
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push(event);
    queueCV.notify_one();
}

#include <type_traits> // std::decay_t, std::is_same_v 등을 위해 필요

void GameSession::BroadcastEvent(const std::shared_ptr<BroadCastEvent>& event) {
    if (!players || players->empty()) return;

    enet_uint32 packetFlags = (event->type == SocketEventType::Move)
                                ? ENET_PACKET_FLAG_UNSEQUENCED
                                : ENET_PACKET_FLAG_RELIABLE;

    ENetPacket* packet = nullptr;

    std::visit([&packet, packetFlags](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            // 페이로드가 없는 경우 처리
        }
        else {
            if (arg) {
                // 과거에 만들어둔 GetDtoBinaryLength와 ToBinary를 200% 활용!
                size_t size = arg->GetDtoBinaryLength();
                std::vector<uint8_t> buffer(size);

                // 버퍼의 시작 주소를 넘겨주어 직렬화 수행
                arg->ToBinary(buffer.data());

                packet = enet_packet_create(buffer.data(), size, packetFlags);
            }
        }
    }, event->payload);

    if (!packet) return;

    // 타겟이 지정되어 있다면 타겟에게만, 아니면 전체에게 브로드캐스트
    bool broadcastToAll = event->target.empty();

    int sentCount = 0;
    for (const auto& [privateKey, player] : *players) {
        if (player.peer != nullptr && player.peer->state == ENET_PEER_STATE_CONNECTED) {

            // 전체 브로드캐스트이거나, 현재 플레이어의 peer가 target 리스트에 있는 경우에만 전송
            if (broadcastToAll || std::find(event->target.begin(), event->target.end(), player.peer) != event->target.end()) {
                enet_peer_send(player.peer, 0, packet);
                sentCount++;
            }
        }
    }

    if (sentCount == 0 && packet->referenceCount == 0) {
        enet_packet_destroy(packet);
    }
}

