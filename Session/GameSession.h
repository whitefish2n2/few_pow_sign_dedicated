#pragma once
#ifndef FPS_SERVER_H
#define FPS_SERVER_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <list>
#include <memory>
#include <queue>
#include <string>
#include <variant>
#include <enet/enet.h>

#include "FhishiX/Vector2.h"
#include "FhishiX/Vector3.h"
#include "../Socket/dto/DefaultDto.h"
#include "../Socket/dto/MoveDto.h"
#include "../Socket/dto/SocketEventType.h"
#include "Dto/MapEnum.h"
#include "Game/Player.h"
#include "Dto/SessionStatus.h"
#include "FhishiX/Vertex.h"
#include "netcode/SessionNetworkDto.h"

using EventPayloadVariant = std::variant<
    std::nullptr_t,
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
    SocketEventType type = Update;
    EventPayloadVariant payload = nullptr;
    ENetPeer* peer = nullptr;
};

struct BroadCastEvent
{
    SocketEventType type = Update;
    BroadCastPayloadVariant payload = nullptr;
    std::vector<ENetPeer*> target;

    BroadCastEvent(SocketEventType type, BroadCastPayloadVariant payload)
    {
        this->type = type;
        this->payload = payload;
    }
    BroadCastEvent(SocketEventType type, BroadCastPayloadVariant payload, const std::vector<ENetPeer*> target)
    {
        this->type = type;
        this->payload = payload;
        this->target = target;
    }
};
class GameSession {
    public:
    std::string sessionId;
    std::uint16_t sessionConnectKey;// 플레이어->세션 연결에 사용하는 ID
    GameSetupBoddari initInfo;
    SESSIONSTATUS status = idle; // 세션 상태
    std::shared_ptr<std::map<uint16_t, Player>> players; // 플레이어 리스트

    MapEnum map;
    std::vector<Vertex> mapVertices;// 맵 버텍스-버텍스 자료 파일을 만들어서 읽어오는식으로 해야할듯

    std::queue<std::shared_ptr<GameEvent>> eventQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;

    void RunAsync();

    Player* RegistUser(const std::string& userKey, ENetPeer* peer);

    void ProcessEvent(const std::shared_ptr<GameEvent>& event);

    void BroadcastEvent(const std::shared_ptr<GameEvent>& event);
    void Start();
    void Stop();
    void Init(std::string sessionId, GameSetupBoddari initInfo);
    bool reset();
    void cleanUp();
    ~GameSession();
    bool running = true;
};


#endif // FPS_SERVER_H

