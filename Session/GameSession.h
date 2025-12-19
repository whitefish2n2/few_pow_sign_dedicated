#pragma once
#ifndef FPS_SERVER_H
#define FPS_SERVER_H

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <queue>
#include <string>
#include <variant>
#include "../Socket/dto/AssignDto.h"
#include "../Socket/dto/DefaultDto.h"
#include "../Socket/dto/MoveDto.h"
#include "../Socket/dto/SocketEventType.h"
#include "Dto/MapEnum.h"
#include "Game/Player.h"
#include "Dto/SessionStatus.h"
#include "Component/Definition/ComponentManager.h"
#include "FhishiX/gameobject/GameObjectManager.h"
#include "Game/Map.h"
#include "netcode/SessionNetworkDto.h"
struct _EnetPeer;
using EventPayloadVariant = std::variant<
    std::nullptr_t,
    std::shared_ptr<AssignRequestDto>,
    std::shared_ptr<DefaultDto>,
    std::shared_ptr<MoveDto>
    // std::shared_ptr<MoveDto> //type-Move
    // 다른 이벤트 DTO 만들어라 훗치훗치
    // TODO 이 마더퍼커 처리해봐
>;
using BroadCastPayloadVariant = std::variant<
    std::nullptr_t
>;


///<summary>
/// 세션에서 처리할 이벤트를 전달하기 위한 구조체에요
/// payload에 type에 맞는 전달 인자를 전달하세요
///</summary>
struct GameEvent {
    uint64_t timestamp = 0;
    SocketEventType type = SocketEventType::Update;
    EventPayloadVariant payload = nullptr;
    ENetPeer* peer = nullptr;
};

struct BroadCastEvent {
    SocketEventType type = SocketEventType::Update;
    BroadCastPayloadVariant payload = nullptr;
    std::vector<ENetPeer*> target;

    BroadCastEvent(SocketEventType type, BroadCastPayloadVariant payload)
        :type(type), payload(payload){}
    BroadCastEvent(SocketEventType type, BroadCastPayloadVariant payload, const std::vector<ENetPeer*>& target)
        : type(type), payload(payload), target(target) {}
};

class GameSession {
    public:
    std::string sessionId;
    std::uint16_t sessionConnectKey;// 플레이어->세션 연결에 사용하는 ID
    GameSetupBoddari initInfo;
    SESSIONSTATUS status = idle; // 세션 상태
    std::shared_ptr<std::map<uint64_t, Player>> players; // 플레이어 리스트
    MapEnum mapType;
    Map map;

    std::queue<std::shared_ptr<GameEvent>> eventQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;

    bool running = true;
    std::thread gameThread; /// 현재 진행중인 세션 스레드
    bool isStopped = false; /// 스레드가 제대로 종료되었는지 확인

    long long int tick;

    ~GameSession();

    void RunAsync();

    void ProcessEventQueue();

    void Tick();

    void SetCharacter(const CharacterSetDto &dto) const;

    std::shared_ptr<Player> RegistUser(const std::string &userKey, ENetPeer *peer) const;

    void ProcessEvent(std::shared_ptr<GameEvent> &event);

    void BroadcastEvent(const std::shared_ptr<GameEvent>& event);
    void Start();
    void Stop();
    void Init(std::string sessionId, GameSetupBoddari initInfo);
    bool reset();
    void cleanUp();
};


#endif // FPS_SERVER_H

