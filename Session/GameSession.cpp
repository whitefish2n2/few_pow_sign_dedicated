#include "GameSession.h"

#include <iostream>
#include <enet/enet.h>
#include <thread>
#include <random>
#include <string>
#include "../Constants.h"
#include "../Socket/dto/AssignDto.h"
#include "../Socket/dto/SocketEventType.h"
#include "../util/util.h"

GameSession::~GameSession() {
    Log("server destroyed");
}

//스레드로 실행
void GameSession::RunAsync() {
    running = true;
    std::thread([this]() {
        this->Start();
    }).detach();
}

void GameSession::ProcessEventQueue() {
    std::unique_lock<std::mutex> lock(queueMutex);
    while (!eventQueue.empty()) {
        if (!running) break;
        std::shared_ptr<GameEvent> e;
        e = eventQueue.front();
        eventQueue.pop();
        lock.unlock();//이벤트 처리동안 unlock
        try
        {
            switch (e->type)
            {
                case Assign: {
                    auto dto = std::get_if<std::shared_ptr<AssignRequestDto>>(&e->payload);
                    break;
                }

                case Input:
                    break;
                case Move:
                {
                    auto dto = (std::get_if<std::shared_ptr<MoveDto>>(&e->payload));
                    if (dto==nullptr) continue;
                    auto secretKey = (*dto)->UserSecretKey;
                    auto inputVector = (*dto)->InputVector;
                    players->at(secretKey).Move(inputVector);
                    break;
                }
                case Setup:
                    break;
                case Update:
                    break;
                case Hit:
                    break;
                case Swap:
                    break;
                case Generate:
                    break;
                case SocketEventType::Default:
                    break;
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
constexpr int tickRateMs = 33;
void GameSession::Start() {
    Log("server is running on port " + std::to_string(Consts::port));
    // Run server loop
    auto lastTickTime = std::chrono::steady_clock::now();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> distrib(0, 255);
    while (running) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTickTime);
        if (elapsed.count()>=tickRateMs) {
            lastTickTime += std::chrono::milliseconds(tickRateMs);
            Tick();
        }else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
        /*while (enet_host_service(server, &event, 10) > 0) {
            std::cout<<event.type<< std::endl;
            서버마다 enet host를 달자-폐기
        }*/
}
void GameSession::Stop() {
    Log("session stopped at " + initInfo.gameId);
    running = false;
}

void GameSession::Init(std::string sessionId, GameSetupBoddari initInfo) {
    this->sessionId = sessionId;
    this->initInfo = initInfo;
    uint64_t privateKey;
    uint8_t publicKey=129;
    initInfo.map;
    //생성위치를 담은 map 클래스를 만들자
    for (auto p : initInfo.players)
    {

        // TODO 이 마더퍼커좀 처리해봐
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<uint64_t> dist(1, 18446744073709551615);

        do {
            privateKey = dist(rng);
        } while (players->contains(privateKey));
        player_status newStatus = player_status(
            0,
            0,
            0,
            Vector3(0,0,0),
            Vector3(0,0,0),
            Vector3(0,0,0)
        );
        Player newPlayer = Player(p.id, p.name,p.key, privateKey, publicKey++, newStatus);
        (*players)[privateKey] = newPlayer;
    }
}

bool GameSession::reset() {
    return false;
}

void GameSession::cleanUp() {
    Stop();
}

std::shared_ptr<Player> GameSession::RegistUser(const std::string &userKey, ENetPeer *peer)
{
    if (!running) return nullptr;
    Player* p = nullptr;
    for(auto& v : *this->players)
    {
        if (v.second.assignKey == userKey)
        {
            p = &v.second;
            break;
        }
    }
    if (p == nullptr) return nullptr;
    p->peer = peer;
    return std::make_shared<Player>(*p);
}

void GameSession::ProcessEvent(const std::shared_ptr<GameEvent>& event)
{
    std::lock_guard<std::mutex> lock(queueMutex);
    eventQueue.push(event);
    queueCV.notify_one();
}

