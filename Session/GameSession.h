#pragma once
#ifndef FPS_SERVER_H
#define FPS_SERVER_H

#include <atomic>
#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <enet/enet.h>

#include "Player.h"
#include "SessionStatus.h"
#include "netcode/SessionNetworkDto.h"

class GameSession {
    public:
    ENetHost* server; // 서버 통신 헤드
    std::string sessionId; // 세션 id
    GameSetupBoddari initInfo;
    SESSIONSTATUS status = idle; // 세션 상태
    std::shared_ptr<std::list<player>> players; // 플레이어 리스트

    void RunAsync();

    void Start();
    void Stop();
    void Init(std::string sessionId, GameSetupBoddari initInfo);
    bool reset();
    void cleanUp();
    ~GameSession();

};


#endif // FPS_SERVER_H

