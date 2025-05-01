#include <csignal>
#include <iostream>
#include <memory>
#include <vector>
#include <enet/enet.h>
#include <thread>

#include "Constants.h"
#include "http-client/DedicateServerNotifier.h"
#include "server-status/ServerStat.h"
#include "util/util.h"
#include "./Session/GameSession.h"
#include "./Session/sessionPool/SessionManager.h"
#include "http-listener/httpRestClient.h"
using std::thread;

#include <iostream>
#include <mutex>
bool isRunning = false;
std::wstring serverUuid;
void onExit(int signal) {
    if (isRunning) {
        isRunning = false;
        std::cout<<"server turn off Code:"<<signal<< std::endl;
        DedicatedServerNotifier::getInstance().notifyDedicatedServerOff(ServerStat::ServerId);
        enet_deinitialize();
    }
}
void inputListener() {
    std::string cmd;
    while (isRunning) {
        std::cin>>cmd;
        if (cmd == "exit" || cmd == "quit") {
            std::cout << "Shutdown command received.\n";
            onExit(SIGINT);
            break;
        }
    }
}
auto updateDelay = 5000;//config 파일을 만들어서 어떻게 잘 받앙봐요
void statusUpdater() {
    while (isRunning) {
        Sleep(updateDelay);
    }
}

//서버 생성 request 처리
int session_counter = 0;
void main_loop() {
    while (true) {
        std::unique_lock<std::mutex> lock(HttpRestClient::getInstance()->queueMutex);

        HttpRestClient::getInstance()->queueCV.wait(lock, [] {
            return !HttpRestClient::getInstance()->requestQueue.empty();
        });

        auto request = HttpRestClient::getInstance()->requestQueue.front();
        HttpRestClient::getInstance()->requestQueue.pop();

        lock.unlock();

        // 세션 생성

        auto newSession = std::make_shared<GameSession>();
        session_counter++;
        newSession->Start();
        SessionManager::getInstance().sessions.push_back(newSession);

        // 작업 완료됐다고 알림
        request->promise.set_value(newSession->sessionId);
    }
}
void inputServerInfo(std::pmr::wstring &key,std::pmr::string &url) {

}
int main() {
    isRunning = false;
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
    std::cout << "enter base url(like this: 123.123.123.123:1421)";
    std::cin>>url;
    std::string val= GenerateUuid();
    serverUuid.assign(val.begin(),val.end());
    DedicatedServerNotifier::getInstance().init(url);
    DedicatedServerNotifier::getInstance().notifyDedicatedServerUp(
        key,
        ip,
        "http://localhost:8888",//서버별 url url을 url해요
        SessionManager::getInstance().sessions);
    isRunning = true;
    std::thread consoleThread(inputListener);
    std::thread statusThread(statusUpdater);
    std::thread httpClientThread(&HttpRestClient::start_http_server,HttpRestClient::getInstance());
    for (auto& t : threads) {
        t.join();
    }
    consoleThread.join();
    onExit(0);
    exit(0);
    return 0;
}

