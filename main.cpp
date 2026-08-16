#include <algorithm>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <thread>

#include "Constants.h"
#include "MonitorUtil.h"
#include "Session/Dto/NetworkStatus.h"
#include "http-client/DedicateServerNotifier.h"
#include "server-status/ServerStat.h"
#include "util/util.h"
#include "./Session/GameSession.h"
#include "./Session/sessionPool/SessionManager.h"
#include "http-listener/httpRestClient.h"
#include <enet/enet.h>

#include "Session/SessionDXViewer/DirectXCore.h"

#include "ServerStatics.h"
#include "PrefabSystem/PrefabManager.h"
#include "Session/Game/MapManager.h"
#include "Session/Game/data/CharacterRegistry.h"
#include "Session/Game/data/WeaponRegistry.h"
#include "Socket/EnetClient.h"
using std::thread;

#ifdef _WIN64
#include "Session/SessionDXViewer/WIN32PROC.h"
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
#endif

std::wstring serverUuid;

void CreateDebugScreen() {
#ifdef _WIN64
    bool expected = false;
    if (!DirectXCore::isViewerAlive.compare_exchange_strong(expected, true)) {
        std::cout << "debug viewer already running (or still closing)\n";
        return;
    }
    std::thread debugViewerThread([]() {
        DirectXCore::isRunningViewer.store(true);
        auto hwnd = CreateDebugWindow(GetModuleHandle(nullptr), 1920, 1080);
        if (DirectXCore::InitD3D(hwnd, 1920, 1080)) {
            DirectXCore::RunDirectXLoop();
        }
        DirectXCore::isViewerAlive.store(false);   // Cleanup까지 끝난 뒤에만 재진입 허용
    });
    debugViewerThread.detach();
#endif
}
void onExit(int signal) {
    if (isRunning.load()) {
        isRunning.store(false);
        std::cout<<"server turn off Code:"<<signal<< std::endl;
        DedicatedServerNotifier::getInstance().notifyDedicatedServerOff(ServerStat::ServerId);
        enet_deinitialize();
#ifdef _WIN64
        timeEndPeriod(1);
#endif
    }
}

struct StatSnapshot {
    double processCpu = 0.0;
    long long processMemoryMb = 0;
    size_t sessionCount = 0;
    int connectedPlayers = 0;
    int liveSessionCount = 0;

    long long avgTps = 0, avgTickUs = 0, avgLagMs = 0;
    double avgThreadCpu = 0.0;
    long long avgBroadphaseUs = 0, avgPairsChecked = 0, avgPairsHit = 0;
    long long avgObjectsAtStart = -1;
    int sessionsWithObjectCount = 0;

    long long avgEventQueueUs = 0, avgUpdateComponentsUs = 0, avgFlushGameObjectUs = 0;
    long long avgBroadcastMovementsUs = 0, avgBroadcastObjectMovementsUs = 0, avgCheckDisconnectedUs = 0;

    long long avgPhysicsIntegrateUs = 0, avgStaticOverlapUs = 0, avgStaticPairsFound = 0, avgNarrowPhaseUs = 0;
    long long avgNarrowPhaseStaticUs = 0, avgNarrowPhaseDynamicUs = 0;
};

///stat 콘솔 명령어와 statAutoLogger가 공유하는 집계 로직 - 여기 하나만 고치면 둘 다 반영됨
StatSnapshot ComputeStatSnapshot() {
    StatSnapshot snap;
    auto sessions = SessionManager::getInstance().getSessionListWeak();
    snap.sessionCount = sessions.size();
    snap.processCpu = GetProcessCpuUsage();
    snap.processMemoryMb = GetProcessMemoryUsageMB();

    long long sumTps = 0, sumTickUs = 0, sumLagMs = 0, sumBroadphaseUs = 0, sumPairsChecked = 0, sumPairsHit = 0;
    long long sumEventQueueUs = 0, sumUpdateComponentsUs = 0, sumFlushGameObjectUs = 0;
    long long sumBroadcastMovementsUs = 0, sumBroadcastObjectMovementsUs = 0, sumCheckDisconnectedUs = 0;
    long long sumPhysicsIntegrateUs = 0, sumStaticOverlapUs = 0, sumStaticPairsFound = 0, sumNarrowPhaseUs = 0;
    long long sumNarrowPhaseStaticUs = 0, sumNarrowPhaseDynamicUs = 0;
    double sumThreadCpu = 0.0;
    long long sumObjectsAtStart = 0;
    int liveSessionCount = 0;
    int sessionsWithObjectCount = 0;
    int totalConnectedPlayers = 0;

    for (auto& weak : sessions) {
        auto s = weak.lock();
        if (!s) continue;
        liveSessionCount++;

        if (s->players) {
            for (auto& [key, p] : *s->players) {
                if (p.status.networkStatus == connected) totalConnectedPlayers++;
            }
        }

        sumTps += s->currentTps.load();
        sumTickUs += s->lastTickMicros.load();
        sumLagMs += s->lagMillis.load();
        sumThreadCpu += s->GetThreadCpuPercent();
        sumBroadphaseUs += s->lastBroadphaseMicros.load();
        sumPairsChecked += s->lastBroadphasePairsChecked.load();
        sumPairsHit += s->lastBroadphasePairsHit.load();

        sumEventQueueUs += s->lastEventQueueMicros.load();
        sumUpdateComponentsUs += s->lastUpdateComponentsMicros.load();
        sumFlushGameObjectUs += s->lastFlushGameObjectMicros.load();
        sumBroadcastMovementsUs += s->lastBroadcastMovementsMicros.load();
        sumBroadcastObjectMovementsUs += s->lastBroadcastObjectMovementsMicros.load();
        sumCheckDisconnectedUs += s->lastCheckDisconnectedMicros.load();

        sumPhysicsIntegrateUs += s->lastPhysicsIntegrateMicros.load();
        sumStaticOverlapUs += s->lastStaticOverlapMicros.load();
        sumStaticPairsFound += s->lastStaticPairsFound.load();
        sumNarrowPhaseUs += s->lastNarrowPhaseMicros.load();
        sumNarrowPhaseStaticUs += s->lastNarrowPhaseStaticMicros.load();
        sumNarrowPhaseDynamicUs += s->lastNarrowPhaseDynamicMicros.load();

        int objAtStart = s->objectCountAtStart.load();
        if (objAtStart >= 0) {
            sumObjectsAtStart += objAtStart;
            sessionsWithObjectCount++;
        }
    }

    snap.connectedPlayers = totalConnectedPlayers;
    snap.liveSessionCount = liveSessionCount;
    snap.sessionsWithObjectCount = sessionsWithObjectCount;
    snap.avgObjectsAtStart = sessionsWithObjectCount > 0 ? sumObjectsAtStart / sessionsWithObjectCount : -1;

    if (liveSessionCount > 0) {
        snap.avgTps = sumTps / liveSessionCount;
        snap.avgTickUs = sumTickUs / liveSessionCount;
        snap.avgLagMs = sumLagMs / liveSessionCount;
        snap.avgThreadCpu = sumThreadCpu / liveSessionCount;
        snap.avgBroadphaseUs = sumBroadphaseUs / liveSessionCount;
        snap.avgPairsChecked = sumPairsChecked / liveSessionCount;
        snap.avgPairsHit = sumPairsHit / liveSessionCount;
        snap.avgEventQueueUs = sumEventQueueUs / liveSessionCount;
        snap.avgUpdateComponentsUs = sumUpdateComponentsUs / liveSessionCount;
        snap.avgFlushGameObjectUs = sumFlushGameObjectUs / liveSessionCount;
        snap.avgBroadcastMovementsUs = sumBroadcastMovementsUs / liveSessionCount;
        snap.avgBroadcastObjectMovementsUs = sumBroadcastObjectMovementsUs / liveSessionCount;
        snap.avgCheckDisconnectedUs = sumCheckDisconnectedUs / liveSessionCount;
        snap.avgPhysicsIntegrateUs = sumPhysicsIntegrateUs / liveSessionCount;
        snap.avgStaticOverlapUs = sumStaticOverlapUs / liveSessionCount;
        snap.avgStaticPairsFound = sumStaticPairsFound / liveSessionCount;
        snap.avgNarrowPhaseUs = sumNarrowPhaseUs / liveSessionCount;
        snap.avgNarrowPhaseStaticUs = sumNarrowPhaseStaticUs / liveSessionCount;
        snap.avgNarrowPhaseDynamicUs = sumNarrowPhaseDynamicUs / liveSessionCount;
    }

    return snap;
}

std::mutex statLogMutex;
std::vector<std::string> statLogBuffer;   // flushStat이 비우고 CSV로 씀

///1초 간격으로 ComputeStatSnapshot()을 CSV 한 줄로 만들어 버퍼에 쌓는 백그라운드 스레드
void statAutoLogger() {
    while (isRunning.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (!isRunning.load()) break;

        StatSnapshot snap = ComputeStatSnapshot();
        long long nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        std::ostringstream row;
        row << nowMs << ','
            << snap.processCpu << ','
            << snap.processMemoryMb << ','
            << snap.sessionCount << ','
            << snap.connectedPlayers << ','
            << snap.liveSessionCount << ','
            << snap.avgTps << ','
            << snap.avgTickUs << ','
            << snap.avgLagMs << ','
            << snap.avgThreadCpu << ','
            << snap.avgBroadphaseUs << ','
            << snap.avgPairsChecked << ','
            << snap.avgPairsHit << ','
            << snap.avgObjectsAtStart << ','
            << snap.avgEventQueueUs << ','
            << snap.avgUpdateComponentsUs << ','
            << snap.avgFlushGameObjectUs << ','
            << snap.avgBroadcastMovementsUs << ','
            << snap.avgBroadcastObjectMovementsUs << ','
            << snap.avgCheckDisconnectedUs << ','
            << snap.avgPhysicsIntegrateUs << ','
            << snap.avgStaticOverlapUs << ','
            << snap.avgStaticPairsFound << ','
            << snap.avgNarrowPhaseUs << ','
            << snap.avgNarrowPhaseStaticUs << ','
            << snap.avgNarrowPhaseDynamicUs;

        std::lock_guard<std::mutex> lock(statLogMutex);
        statLogBuffer.push_back(row.str());
    }
}

void inputListener() {
    std::string cmd;
    while (isRunning.load()) {
        std::cin>>cmd;
        if (cmd == "exit" || cmd == "quit") {
            std::cout << "Shutdown command received.\n";
            onExit(SIGINT);
            break;
        }
        else if (cmd == "debug") {
            CreateDebugScreen();
        }
        else if (cmd == "debugoff") {

        }
        else if (cmd == "help" || cmd == "?") {
            std::cout << "===== Available Commands =====\n"
                      << "  exit / quit      - 서버 정상 종료(닷지 통보 후 종료)\n"
                      << "  debug            - 디버그 뷰어(DirectX 시각화 창) 실행\n"
                      << "  debugoff         - (미구현)\n"
                      << "  stat             - 전체 세션 평균 성능 지표 1회 출력(TPS/틱시간/Lag/CPU/브로드페이즈/틱 구간별 소요시간)\n"
                      << "  flushStat        - 1초 간격으로 자동 수집 중인 stat 데이터를 stat_log.csv에 append\n"
                      << "  statComponent    - 컴포넌트 타입별 UpdateAll() 소요시간을 전체 세션 평균으로 출력(내림차순 정렬)\n"
                      << "  help / ?         - 이 도움말 출력\n";
        }
        else if (cmd == "stat") {
            StatSnapshot snap = ComputeStatSnapshot();

            std::cout << "===== Process ====="
                      << " CPU:" << snap.processCpu << "%"
                      << " MemoryMB:" << snap.processMemoryMb
                      << " Sessions:" << snap.sessionCount
                      << " ConnectedPlayers:" << snap.connectedPlayers
                      << std::endl;

            if (snap.liveSessionCount == 0) {
                std::cout << "(no live sessions)" << std::endl;
            } else {
                std::cout << "===== Session Averages (n=" << snap.liveSessionCount << ") ====="
                          << " TPS:" << snap.avgTps
                          << " TickTime:" << snap.avgTickUs << "us"
                          << " Lag:" << snap.avgLagMs << "ms"
                          << " CPU:" << snap.avgThreadCpu << "%"
                          << " Broadphase:" << snap.avgBroadphaseUs << "us("
                          << snap.avgPairsChecked << " checked/"
                          << snap.avgPairsHit << " hit)"
                          << " ObjectsAtStart:" << snap.avgObjectsAtStart
                          << " (measured in " << snap.sessionsWithObjectCount << "/" << snap.liveSessionCount << " sessions)"
                          << std::endl;
                std::cout << "===== Tick Breakdown (avg) ====="
                          << " EventQueue:" << snap.avgEventQueueUs << "us"
                          << " UpdateComponents:" << snap.avgUpdateComponentsUs << "us"
                          << " FlushGameObject:" << snap.avgFlushGameObjectUs << "us"
                          << " BroadcastMovements:" << snap.avgBroadcastMovementsUs << "us"
                          << " BroadcastObjectMovements:" << snap.avgBroadcastObjectMovementsUs << "us"
                          << " CheckDisconnected:" << snap.avgCheckDisconnectedUs << "us"
                          << std::endl;
                std::cout << "===== Physics Breakdown (avg) ====="
                          << " Integrate:" << snap.avgPhysicsIntegrateUs << "us"
                          << " StaticOverlap:" << snap.avgStaticOverlapUs << "us(" << snap.avgStaticPairsFound << " pairs)"
                          << " DynamicBroadphase:" << snap.avgBroadphaseUs << "us"
                          << " NarrowPhase:" << snap.avgNarrowPhaseUs << "us(static:" << snap.avgNarrowPhaseStaticUs
                          << "us/dynamic:" << snap.avgNarrowPhaseDynamicUs << "us)"
                          << std::endl;
            }
        }
        else if (cmd == "flushStat") {
            std::vector<std::string> toWrite;
            {
                std::lock_guard<std::mutex> lock(statLogMutex);
                toWrite.swap(statLogBuffer);
            }
            if (toWrite.empty()) {
                std::cout << "stat log buffer is empty." << std::endl;
            } else {
                std::ifstream check("stat_log.csv");
                bool needsHeader = !check.good() || check.peek() == std::ifstream::traits_type::eof();
                check.close();

                std::ofstream file("stat_log.csv", std::ios::app);
                if (needsHeader) {
                    file << "timestampMs,processCpu,processMemoryMb,sessions,connectedPlayers,liveSessions,"
                         << "avgTps,avgTickUs,avgLagMs,avgThreadCpu,avgBroadphaseUs,avgPairsChecked,avgPairsHit,avgObjectsAtStart,"
                         << "avgEventQueueUs,avgUpdateComponentsUs,avgFlushGameObjectUs,avgBroadcastMovementsUs,avgBroadcastObjectMovementsUs,avgCheckDisconnectedUs,"
                         << "avgPhysicsIntegrateUs,avgStaticOverlapUs,avgStaticPairsFound,avgNarrowPhaseUs,"
                         << "avgNarrowPhaseStaticUs,avgNarrowPhaseDynamicUs\n";
                }
                for (auto& row : toWrite) file << row << "\n";
                file.close();
                std::cout << "Flushed " << toWrite.size() << " rows to stat_log.csv" << std::endl;
            }
        }
        else if (cmd == "statComponent") {
            auto sessions = SessionManager::getInstance().getSessionListWeak();
            std::unordered_map<std::string, std::pair<long long, int>> sums; // 타입이름 -> (합, 표본수)

            for (auto& weak : sessions) {
                auto s = weak.lock();
                if (!s || !s->componentManager) continue;
                for (auto& [name, micros] : s->componentManager->GetUpdateBreakdown()) {
                    auto& entry = sums[name];
                    entry.first += micros;
                    entry.second += 1;
                }
            }

            std::vector<std::pair<std::string, long long>> averages;
            averages.reserve(sums.size());
            for (auto& [name, sumCount] : sums) {
                long long avg = sumCount.second > 0 ? sumCount.first / sumCount.second : 0;
                averages.emplace_back(name, avg);
            }
            std::sort(averages.begin(), averages.end(),
                      [](const auto& a, const auto& b) { return a.second > b.second; });

            std::cout << "===== Component UpdateAll Averages (across " << sessions.size() << " sessions) =====" << std::endl;
            for (auto& [name, avg] : averages) {
                std::cout << "  " << name << ": " << avg << "us" << std::endl;
            }
        }
    }
}

auto updateDelay = 5000;//config 파일을 만들어서 어떻게 잘 받앙봐요
void statusUpdater() {
    while (isRunning.load()) {
        Sleep(updateDelay);
    }
}
#ifdef _WIN64
LONG WINAPI CrashHandler(EXCEPTION_POINTERS* info)
{
    onExit(2);
    std::cout<<info<<std::endl;
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main() {
    #ifdef _WIN64
    SetUnhandledExceptionFilter(CrashHandler);
    std::signal(SIGTERM, onExit);
    #endif
    try {
        isRunning.store(false);
#ifdef _WIN64
        timeBeginPeriod(1);   // Windows 기본 타이머 해상도(~15.6ms)를 1ms로 고정 - 세션 틱루프의 sleep_for 정밀도가 다른 프로세스 상태에 좌우되지 않게
#endif
        if (enet_initialize() != 0) {
            std::cerr << "ENet init failed\n";
            return 1;
        }
        //프리팹 로드
        std::string prefabDirectory = "./Prefabs";
        PrefabManager::Init(prefabDirectory);

        //캐릭터 로드
        CharacterRegistry::Init("./Assets/CharacterData.json");

        //무기 로드
        WeaponRegistry::Init("./Assets/WeaponData.json");

        std::signal(SIGINT, onExit);   // Ctrl+C
        std::signal(SIGTERM, onExit);
        const std::string ip = GetLocalIP();
        std::vector<std::thread> threads;

        char* key = nullptr;
        size_t len = 0;
        _dupenv_s(&key,&len,"HolyMolyIsGodDamnSecretKey");
        if (!key) {
            std::cerr << "Environment variable SPRING_CONNECT_KEY not set!" << std::endl;
            return 1;
        }
        std::cout << "key"<<key<< std::endl;
        std::string matchServerUrl;
        std::cout << "enter match server base address(like this: 123.123.123.123:25565): ";
        matchServerUrl = "localhost:25565";
        std::string val= GenerateUuid();
        serverUuid.assign(val.begin(),val.end());


        DedicatedServerNotifier::getInstance().init(matchServerUrl);
        std::vector<std::shared_ptr<GameSession>> sessions;
        for (const auto& [k, v] : SessionManager::getInstance().sessions) {
            sessions.push_back(v);
        }
        DedicatedServerNotifier::getInstance().notifyDedicatedServerUp(
        key,
            ip,
            "http://" + ip + ":" + std::to_string(Consts::httpPort),//이 서버 http url
            std::to_string(Consts::udpPort),//이 서버 udp 포트
            sessions
        );
        isRunning = true;
        std::thread consoleThread(inputListener);
        std::thread statusThread(statusUpdater);
        std::thread statLoggerThread(statAutoLogger);
        statLoggerThread.detach();
        std::thread httpClientThread(&HttpRestClient::start_http_server, HttpRestClient::getInstance());
        std::thread enetThread(&EnetClient::RunClient, EnetClient::GetInstance(), Consts::udpPort);
        std::thread reaperThread([]() {   // 유령 세션 리퍼
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                SessionManager::getInstance().reapStoppedSessions();
            }
        });

        /*Initialize Flow*/

        MapRegister::Init("Assets/MapInfo.json");
        MapManager::GetInstance()->Init();

        CollisionSolver::Initialize();
#ifdef _WIN64
        CreateDebugScreen();
#endif

        /*Initialize Flow end*/
        for (auto& t : threads) {
            t.join();
        }
        consoleThread.join();
        onExit(0);
        exit(0);
        return 0;
    }
    catch (const std::exception e) {
        onExit(1);
        std::cout << e.what()<<std::endl;
    }
}




