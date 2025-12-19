#include "GameSession.h"

#include <iostream>
#include <enet/enet.h>
#include <thread>
#include <random>
#include <string>
#include <ranges>

#include "SessionContext.h"
#include "Game/Player.h"
#include "SessionUtil.h"
#include "../Constants.h"
#include "../ServerStatics.h"
#include "../Socket/dto/AssignDto.h"
#include "../Socket/dto/SocketEventType.h"
#include "../util/util.h"
;

GameSession::~GameSession() {
    Log("server destroyed");
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
                    auto dto = *(std::get_if<std::shared_ptr<AssignRequestDto>>(&e->payload));
                        for (const auto& val: *players | std::views::values) {
                        if (val.assignKey == dto->Key){
                            //val.peer = dto->

                        }
                    }
                    //todo
                    break;
                }

                case SocketEventType::Input:
                    break;
                case SocketEventType::Move:
                {
                    auto dto = (std::get_if<std::shared_ptr<MoveDto>>(&e->payload));
                    if (dto==nullptr) continue;
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
constexpr int tickRateMs = 33;
void GameSession::Start() {
    Log("session is running on port " + std::to_string(Consts::port));
    // Run server loop

    // 게임 내(스레드 내부) 전역 매니저 인스턴스 초기화, 생성
    gameSessionInstance = this;
    gameObjectManagerInstance = new GameObjectManager();
    componentManagerInstance = new ComponentManager();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> distrib(0, 255);
    auto previousTime = std::chrono::steady_clock::now();
    double lag = 0.0;
    while (isRunning.load() and running) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - previousTime);
        previousTime = now;
        lag += elapsed.count();
        while (lag>=tickRateMs) {
            Tick();
            lag -= tickRateMs;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
        /*while (enet_host_service(server, &event, 10) > 0) {
            std::cout<<event.type<< std::endl;
            서버마다 enet host를 달자-폐기
        }*/
}
void GameSession::Stop() {
    Log("session stopped id: " + initInfo.gameId);
    running = false;
    if (gameThread.joinable()) {
        gameThread.join();
    }
    isStopped = true;
}

void GameSession::Init(std::string sessionId, GameSetupBoddari initInfo) {
    this->players = std::make_shared<std::map<uint64_t, Player>>();
    this->sessionId = sessionId;
    this->initInfo = initInfo;
    uint64_t privateKey;
    uint8_t publicKey=129;
    //todo: 생성위치를 담은 map 클래스를 만들자
    std::cout<<"New Session Enqueue Players:"<< std::endl;
    for (auto p : initInfo.players)
    {
        std::cout<<p.id<<std::endl;

        // TODO 이좀 처리해봐
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<uint64_t> dist(1, 18446744073709551615);

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

