#pragma once
#ifndef FPS_SERVER_H
#define FPS_SERVER_H
#define WIN32_LEAN_AND_MEAN

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <queue>
#include <shared_mutex>
#include <string>
#include <utility>
#include <variant>

#pragma push_macro("U")
#undef U
#include <readerwriterqueue/readerwriterqueue.h>
#pragma pop_macro("U")
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
struct JumpDto;
struct RespawnDto;
struct HitDto;
struct DeathDto;
struct BroadcastPlayerMoveDto;
struct GenerateDto;
struct ProgressNotifyDto;
struct LoadingProgressDto;
struct GameEvent;
struct MapInitDto;
struct InteractDto;
struct DropWeaponDto;
struct SwapWeaponDto;
struct ReloadDto;
struct ShotDto;
struct HitThisDto;
struct HitStructureDto;
struct GetWeaponNotifyDto;
struct DropWeaponNotifyDto;
struct SwapWeaponNotifyDto;
struct ReloadNotifyDto;
struct ShotNotifyDto;
struct PhaseChangeNotifyDto;
struct GameEndNotifyDto;
struct RoundEndNotifyDto;
class Weapon;
class Collider;
class Renderer;
struct Mesh;
struct BroadcastMoveDto;
struct GenerateObjectDto;
class GameObject;
struct Vector3;
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
    std::unique_ptr<LoadingProgressDto, void(*)(LoadingProgressDto*)>,
    std::unique_ptr<InteractDto, void(*)(InteractDto*)>,
    std::unique_ptr<DropWeaponDto, void(*)(DropWeaponDto*)>,
    std::unique_ptr<SwapWeaponDto, void(*)(SwapWeaponDto*)>,
    std::unique_ptr<ReloadDto, void(*)(ReloadDto*)>,
    std::unique_ptr<JumpDto, void(*)(JumpDto*)>,
    std::unique_ptr<ShotDto, void(*)(ShotDto*)>,
    std::unique_ptr<HitThisDto, void(*)(HitThisDto*)>,
    std::unique_ptr<HitStructureDto, void(*)(HitStructureDto*)>
    // std::unique_ptr<MoveDto> //type-Move

>;
using GameEventPtr = std::unique_ptr<GameEvent, void(*)(GameEvent*)>;
using BroadCastPayloadVariant = std::variant<
    std::nullptr_t,
    std::unique_ptr<BroadcastMoveDto, void(*)(BroadcastMoveDto*)>,
    std::unique_ptr<MapInitDto, void(*)(MapInitDto*)>,
    std::unique_ptr<ProgressNotifyDto, void(*)(ProgressNotifyDto*)>,
    std::unique_ptr<AssignResponseDto, void(*)(AssignResponseDto*)>,
    std::unique_ptr<GenerateDto, void(*)(GenerateDto*)>,
    std::unique_ptr<BroadcastPlayerMoveDto, void(*)(BroadcastPlayerMoveDto*)>,
    std::unique_ptr<RespawnDto, void(*)(RespawnDto*)>,
    std::unique_ptr<HitDto, void(*)(HitDto*)>,
    std::unique_ptr<DeathDto, void(*)(DeathDto*)>,
    std::unique_ptr<GetWeaponNotifyDto, void(*)(GetWeaponNotifyDto*)>,
    std::unique_ptr<DropWeaponNotifyDto, void(*)(DropWeaponNotifyDto*)>,
    std::unique_ptr<SwapWeaponNotifyDto, void(*)(SwapWeaponNotifyDto*)>,
    std::unique_ptr<ReloadNotifyDto, void(*)(ReloadNotifyDto*)>,
    std::unique_ptr<GenerateObjectDto, void(*)(GenerateObjectDto*)>,
    std::unique_ptr<ShotNotifyDto, void(*)(ShotNotifyDto*)>,
    std::unique_ptr<PhaseChangeNotifyDto, void(*)(PhaseChangeNotifyDto*)>,
    std::unique_ptr<GameEndNotifyDto, void(*)(GameEndNotifyDto*)>,
    std::unique_ptr<RoundEndNotifyDto, void(*)(RoundEndNotifyDto*)>
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
private:

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
    LayerMask _playerMask;
    LayerMask _groundMask;
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

    moodycamel::ReaderWriterQueue<GameEventPtr> eventQueue;

    std::unique_ptr<GameObjectManager> objectManager;
    std::unique_ptr<ComponentManager> componentManager;

    std::vector<std::pair<std::string,int>> objectNameToIdCahce;




    bool running = true;
    std::thread gameThread; /// 현재 진행중인 세션 스레드
    bool isStopped = false; /// 스레드가 제대로 종료되었는지 확인

    // 틱 페이싱 상태 (Start() 로컬변수 -> 멤버로 이동)
    std::chrono::steady_clock::time_point previousTickTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point tpsWindowStart = previousTickTime;
    std::chrono::nanoseconds lag{0};

    void TryTick();

    int abandonedTicks = 0;   /// 접속자 0 지속 감시

    long long int tick = 0;

    /// stat 콘솔 명령용 계측치 - 틱 루프 스레드가 갱신, 콘솔 스레드가 읽음(모두 atomic)
    std::atomic<int> currentTps{0};       /// 최근 1초간 확정된 TPS
    std::atomic<int> tpsTickCounter{0};   /// 현재 집계 중인 초의 tick 카운트(내부용)
    std::atomic<long long> lastTickMicros{0};  /// 최근 Tick() 1회 실행시간(us)
    std::atomic<long long> lagMillis{0};       /// 현재 누적 lag(ms) - 못 따라잡고 밀린 정도

    /// 직전 GetThreadCpuPercent() 호출 시점 대비 방금까지의 CPU 사용률(%) - 작업관리자와 동일한 "최근 구간" 값.
    /// 첫 호출(스냅샷 없음)은 스레드 생성 이후 누적 평균으로 대체. 계산 불가 시 -1.
    double GetThreadCpuPercent();
private:
    unsigned long long _prevCpuTime100ns = 0;
    unsigned long long _prevWallTime100ns = 0;
    bool _hasCpuSnapshot = false;
public:

    /// 다이나믹-다이나믹 브로드페이즈(현재 O(n^2) 전수비교) 구간만 격리한 계측치 - PhysicsManager가 매 틱 갱신
    std::atomic<long long> lastBroadphaseMicros{0};       /// 해당 구간 소요시간(us)
    std::atomic<long long> lastBroadphasePairsChecked{0}; /// 이번 틱에 실제로 비교한 쌍의 수(n*(n-1)/2)
    std::atomic<long long> lastBroadphasePairsHit{0};     /// 그중 실제로 AABB가 겹친 쌍의 수

    /// 게임 시작 시(전원 스폰 완료 시점) 1회만 측정되는 세션 오브젝트 수. 미측정 시 -1.
    std::atomic<int> objectCountAtStart{-1};

    /// PhysicsManager::PhysicsUpdate() 내부 세부 구간 계측 - 다이나믹-다이나믹 브로드페이즈(위) 말고 나머지 구간들
    std::atomic<long long> lastPhysicsIntegrateMicros{0};   /// Rigidbody Integrate + dynamicProxies 구성
    std::atomic<long long> lastStaticOverlapMicros{0};      /// KD-트리로 다이나믹 vs 스태틱 오버랩 쿼리
    std::atomic<long long> lastStaticPairsFound{0};         /// 그 결과로 찾은 다이나믹-스태틱 후보쌍 수
    std::atomic<long long> lastNarrowPhaseMicros{0};        /// 충돌 판정+해결(솔버, 최대 4패스) 전체
    std::atomic<long long> lastNarrowPhaseStaticMicros{0};  /// 그중 "다이나믹 vs 스태틱" 루프만(4패스 합산)
    std::atomic<long long> lastNarrowPhaseDynamicMicros{0}; /// 그중 "다이나믹 vs 다이나믹" 루프만(4패스 합산)

    /// Tick() 내부 6단계 각각의 최근 1회 실행시간(us) - 어디서 시간이 드는지 구간별로 확인용
    std::atomic<long long> lastEventQueueMicros{0};
    std::atomic<long long> lastUpdateComponentsMicros{0};
    std::atomic<long long> lastFlushGameObjectMicros{0};
    std::atomic<long long> lastBroadcastMovementsMicros{0};
    std::atomic<long long> lastBroadcastObjectMovementsMicros{0};
    std::atomic<long long> lastCheckDisconnectedMicros{0};

    ~GameSession();
    GameSession();

    void ProcessEventQueue();

    void Tick();

    void UpdateComponents() const;

    void FlushGameObject() const;

    void SetCharacter(const CharacterSetDto &dto) const;

    void IHitValidator(HitThisDto *hitThisDto, Player *shooter, Player *target, Weapon *weapon);

    /// 플레이어가 아닌 오브젝트(벽 등)를 실제로 맞춘 게 확정된 뒤 호출 — 리지드바디에 넉백 임펄스 적용
    void ApplyKnockback(Collider *hitCollider, const Vector3 &hitPoint, const Vector3 &dir, Weapon *weapon);

    void CheckAllPlayerDisconnected();

    std::shared_ptr<Player> RegistUser(const std::string &userKey, ENetPeer *peer) const;

    void ProcessEvent(GameEventPtr event);

    void BroadcastEvent(const std::shared_ptr<BroadCastEvent>& event);
    void BroadcastMovements();
    void BroadcastObjectMovements();

    GameObject SpawnSyncObject(uint32_t prefabId, const Vector3 &pos);

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

