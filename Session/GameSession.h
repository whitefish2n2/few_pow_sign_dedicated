#pragma once
#ifndef FPS_SERVER_H
#define FPS_SERVER_H
#define WIN32_LEAN_AND_MEAN

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <queue>
#include <shared_mutex>
#include <string>
#include <variant>
#include "../Socket/dto/AssignDto.h"
#include "../Socket/dto/DefaultDto.h"
#include "../Socket/dto/MoveDto.h"
#include "../Socket/dto/SocketEventType.h"
#include "Dto/MapInfo.h"
#include "Game/Player.h"
#include "Dto/SessionStatus.h"
#include "Game/PhysicsSystem.h"
#include "netcode/SessionNetworkDto.h"
struct _EnetPeer;
class GameObjectManager;
class ComponentManager;
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
#ifdef _WIN64
    std::vector<Renderer> renderers = {};
    std::queue<int> usableRenderersIndex = {};
#endif
    public:
    std::string sessionId;
    std::uint16_t sessionConnectKey;// 플레이어->세션 연결에 사용하는 ID
    GameSetupBoddari initInfo;
    SESSIONSTATUS status = idle; // 세션 상태
    std::shared_ptr<std::map<uint64_t, Player>> players; // 플레이어 리스트
    MapInfo mapType;
    PhysicsSystem map;

    std::queue<std::shared_ptr<GameEvent>> eventQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;

    std::unique_ptr<GameObjectManager> objectManager;
    std::unique_ptr<ComponentManager> componentManager;



    bool running = true;
    std::thread gameThread; /// 현재 진행중인 세션 스레드
    bool isStopped = false; /// 스레드가 제대로 종료되었는지 확인

    long long int tick;

    ~GameSession();
    GameSession();

    void RunAsync();

    void ProcessEventQueue();

    void Tick();

    void SetCharacter(const CharacterSetDto &dto) const;

    std::shared_ptr<Player> RegistUser(const std::string &userKey, ENetPeer *peer) const;

    void ProcessEvent(std::shared_ptr<GameEvent> &event);

    void BroadcastEvent(const std::shared_ptr<GameEvent>& event);
    void Start();
    void Stop();
    void Init(const std::string &sessionId, const GameSetupBoddari &initInfo);
    bool reset();
    void cleanUp();
#ifdef _WIN64
    ///Renderer를 사용하는 객체에서 여기에 렌더러를 등록하면 SessionDxViewer에서 렌더링할때 렌더링됨.렌더러가 저장된 인덱스를 반환함.
    void InsertRenderer(Renderer renderer);
    ///Renderer를 사용하는 객체에서 등록한 렌더러를 삭제할 때 렌더러가 저장된 인덱스(InsertRenderer호출시 반환)를 매개변수로 넣으면 해당 렌더러가 삭제됨.
    void DeleteRenderer(int index);
    const std::vector<Renderer>* GetRenderers() const { return &renderers;};
#endif
};


#endif // FPS_SERVER_H

