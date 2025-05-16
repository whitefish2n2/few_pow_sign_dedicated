//
// Created by user on 25. 4. 27.
//

#include "SessionManager.h"

#include <mutex>
#include <random>
#include <utility>

///
/// @param initInfo Session에 전달되어 session 내부에서 처리됨요
/// @return 새롭게 생성한 세션의 식별 id를 반환합니다
uint16_t SessionManager::makeNewSession(GameSetupBoddari initInfo){
    auto newSession = std::make_shared<GameSession>();

    uint16_t sessionKey = sessionKeyRoundRobin | std::random_device{}();;
    while (sessions.contains(sessionKey))
    {
        sessionKeyRoundRobin++;
        sessionKey = sessionKeyRoundRobin | std::random_device{}();
    }
    newSession->Init(std::to_string(sessionKey),std::move(initInfo));
    newSession->RunAsync();
    sessions[sessionKey] = newSession;
    return sessionKey;
}


void SessionManager::cleanupSessions() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& session : sessions) {
        session.second->cleanUp();//delete
    }
    sessions.clear();
}

