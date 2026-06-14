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
#include <utility>
#include <variant>

#include "Time.h"
#include "../Socket/dto/AssignDto.h"
#include "../Socket/dto/AssignResponseDto.h"
#include "../Socket/dto/DefaultDto.h"
#include "../Socket/dto/MoveDto.h"
#include "../Socket/dto/SocketEventType.h"
#include "Dto/MapInfo.h"
#include "Game/Player.h"
#include "Dto/SessionStatus.h"
#include "netcode/SessionNetworkDto.h"
struct ProgressNotifyDto;
struct LoadingProgressDto;
struct GameEvent;
struct MapInitDto;
class Renderer;
struct Mesh;
class BroadcastMoveDto;
struct _EnetPeer;
class GameObjectManager;
class ComponentManager;
class PhysicsSystem;
using EventPayloadVariant = std::variant<
    std::monostate,
    std::unique_ptr<AssignRequestDto, void(*)(AssignRequestDto*)>,
    std::unique_ptr<DefaultDto, void(*)(DefaultDto*)>,
    std::unique_ptr<MoveDto, void(*)(MoveDto*)>,
    std::unique_ptr<MapInitDto, void(*)(MapInitDto*)>,
    std::unique_ptr<LoadingProgressDto, void(*)(LoadingProgressDto*)>
    // std::unique_ptr<MoveDto> //type-Move
    // 다른 이벤트 DTO 만들어라 훗치훗치
    // TODO 이 마더퍼커 처리해봐
>;
using GameEventPtr = std::unique_ptr<GameEvent, void(*)(GameEvent*)>;
using BroadCastPayloadVariant = std::variant<
    std::nullptr_t,
    std::unique_ptr<BroadcastMoveDto, void(*)(BroadcastMoveDto*)>,
    std::unique_ptr<MapInitDto, void(*)(MapInitDto*)>,
    std::unique_ptr<ProgressNotifyDto, void(*)(ProgressNotifyDto*)>,
    std::unique_ptr<AssignResponseDto, void(*)(AssignResponseDto*)>
>;


///<summary>
/// 세션에서 처리할 이벤트를 전달하기 위한 구조체에요
/// payload에 type에 맞는 전달 인자를 전달하세요
///</summary>
struct GameEvent {
    uint64_t timestamp = 0;
    SocketEventType type = SocketEventType::Update;
    EventPayloadVariant payload = std::monostate{};
    ENetPeer* peer = nullptr;
};

struct BroadCastEvent {
    SocketEventType type = SocketEventType::Update;
    BroadCastPayloadVariant payload = nullptr;

    std::vector<ENetPeer*> target;

    uint8_t* Serialize() {

    }
    BroadCastEvent(SocketEventType type, BroadCastPayloadVariant payload)
        :type(type), payload(std::move(payload)){}
    BroadCastEvent(SocketEventType type, BroadCastPayloadVariant payload, const std::vector<ENetPeer*>& target)
        : type(type), payload(std::move(payload)), target(target) {}
    BroadCastEvent() = default;
};

///<summary>
///렌더링 스레드에 전달하기 위한 패킷 구조체에요
///</summary>
#ifdef _WIN64
#include <DirectXMath.h>
struct RenderPacket {
    Mesh* mesh;
    DirectX::XMFLOAT4X4 worldMatrix;
    DirectX::XMFLOAT4 color;
    bool isWireframe;
};
#endif

///게임 세션 구조체(게임 단위)
class GameSession {
#ifdef _WIN64
    std::vector<Renderer> renderers = {};
    std::queue<int> usableRenderersIndex = {};
    std::atomic<bool> isRenderDataReady = false;
    //렌더러 3중 버퍼링 배열
    std::vector<RenderPacket> buffers[3];

    // 각 스레드가 사용할 버퍼의 인덱스
    int writeIdx = 0; // 로직 스레드가 쓰는 용도
    int readIdx = 1;  // 렌더 스레드가 읽는 용도
    int nextIdx = 2;  // 완성된 데이터가 대기하는 용도

    std::mutex renderBufferMutex;
#endif
    public:
    std::string sessionId;

    std::string& getSessionId(){return sessionId;};
    std::uint16_t sessionConnectKey;// 플레이어->세션 연결에 사용하는 ID
    GameSetupBoddari initInfo;
    SESSIONSTATUS status = idle; // 세션 상태
    std::shared_ptr<std::map<uint64_t, Player>> players; // 플레이어 리스트
    MapInfo mapType;
    std::unique_ptr<PhysicsSystem> physicsSystem;

    Time time;

    std::queue<GameEventPtr> eventQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;

    std::unique_ptr<GameObjectManager> objectManager;
    std::unique_ptr<ComponentManager> componentManager;

    std::vector<std::pair<std::string,int>> objectNameToIdCahce;

    bool running = true;
    std::thread gameThread; /// 현재 진행중인 세션 스레드
    bool isStopped = false; /// 스레드가 제대로 종료되었는지 확인

    long long int tick;

    ~GameSession();
    GameSession();

    void RunAsync();

    void ProcessEventQueue();

    void Tick();

    void UpdateComponents() const;

    void FlushGameObject() const;

    void SetCharacter(const CharacterSetDto &dto) const;

    std::shared_ptr<Player> RegistUser(const std::string &userKey, ENetPeer *peer) const;

    void ProcessEvent(GameEventPtr event);

    void BroadcastEvent(const std::shared_ptr<BroadCastEvent>& event);
    void Start();
    void Stop();
    void Init(const std::string &sessionId, const GameSetupBoddari &initInfo);
    bool reset();
    void cleanUp();
#ifdef _WIN64

    void UpdateRenderBuffer();

    ///Renderer를 사용하는 객체에서 여기에 렌더러를 등록하면 SessionDxViewer에서 렌더링할때 렌더링됨.렌더러가 저장된 인덱스를 반환함.
    int InsertRenderer(const Renderer &renderer);
    ///Renderer를 사용하는 객체에서 등록한 렌더러를 삭제할 때 렌더러가 저장된 인덱스(InsertRenderer호출시 반환)를 매개변수로 넣으면 해당 렌더러가 삭제됨.
    void DeleteRenderer(int index);

    const std::vector<RenderPacket> *GetRenderPackets();
#endif
};

#endif // FPS_SERVER_H

