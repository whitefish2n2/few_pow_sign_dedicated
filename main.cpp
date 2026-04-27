#include <csignal>
#include <iostream>
#include <memory>
#include <vector>

#include <thread>

#include "Constants.h"
#include "http-client/DedicateServerNotifier.h"
#include "server-status/ServerStat.h"
#include "util/util.h"
#include "./Session/GameSession.h"
#include "./Session/sessionPool/SessionManager.h"
#include "http-listener/httpRestClient.h"
#include <enet/enet.h>

#include "Session/SessionDXViewer/DirectXCore.h"

#include "ServerStatics.h"
#include "Session/Game/MapManager.h"
using std::thread;

#ifdef _WIN64
#include "Session/SessionDXViewer/WIN32PROC.h"
#endif

std::wstring serverUuid;

void CreateDebugScreen() {
#ifdef _WIN64
    std::thread debugViewerThread([]() {
        auto hwnd = CreateDebugWindow(GetModuleHandle(nullptr), 1920, 1080);
        if (DirectXCore::InitD3D(hwnd, 1920, 1080)) {
            DirectXCore::RunDirectXLoop();
        }
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
        if (enet_initialize() != 0) {
            std::cerr << "ENet init failed\n";
            return 1;
        }
        std::signal(SIGINT, onExit);   // Ctrl+C
        std::signal(SIGTERM, onExit);
        const std::string ip = GetLocalIP();
        std::vector<std::thread> threads;

        std::string url;//key for connect to spring server, write url like this : 123.123.123.123:8080
        char* key = nullptr;
        size_t len = 0;
        _dupenv_s(&key,&len,"HolyMolyIsGodDamnSecretKey");
        if (!key) {
            std::cerr << "Environment variable SPRING_CONNECT_KEY not set!" << std::endl;
            return 1;
        }
        std::cout << "key"<<key<< std::endl;
        std::cout << "enter match server base address(like this: 123.123.123.123:1421)";
        url = "localhost:25565";
        std::string val= GenerateUuid();
        serverUuid.assign(val.begin(),val.end());


        DedicatedServerNotifier::getInstance().init(url);
        std::vector<std::shared_ptr<GameSession>> sessions;
        for (const auto& [k, v] : SessionManager::getInstance().sessions) {
            sessions.push_back(v);
        }
        DedicatedServerNotifier::getInstance().notifyDedicatedServerUp(
            key,
            ip,
            "http://localhost:8888",//todo 서버별 url url을 url해요
            sessions
        );
        isRunning = true;
        std::thread consoleThread(inputListener);
        std::thread statusThread(statusUpdater);
        std::thread httpClientThread(&HttpRestClient::start_http_server, HttpRestClient::getInstance());
        std::cout << "1"<< std::endl;
        /*Initialize Flow*/

        MapRegister::Init("Assets/MapInfo.json");
        MapManager::GetInstance()->Init();

        CollisionSolver::Initialize();
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




