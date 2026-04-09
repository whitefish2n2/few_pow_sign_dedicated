//
// Created by user on 25. 4. 27.
//

#include "SessionManager.h"

#include <mutex>
#include <random>
#include <shared_mutex>
#include <utility>


std::vector<std::weak_ptr<GameSession>> SessionManager::getSessionListWeak() {
    std::shared_lock lock(_sessionsLock);

    std::vector<std::weak_ptr<GameSession>> sessionsCopy;

    sessionsCopy.reserve(sessions.size());

    for (const auto& val : sessions | std::views::values) {
        sessionsCopy.push_back(val);
    }

    return sessionsCopy;
}

///
/// @param initInfo Session에 전달되어 session 내부에서 처리됨요
/// @return 새롭게 생성한 세션의 식별 id를 반환합니다
uint16_t SessionManager::makeNewSession(GameSetupBoddari initInfo){
    auto newSession = std::make_shared<GameSession>();
    uint16_t sessionKey = 0;
    {

        std::unique_lock lock(_sessionsLock);

        thread_local std::mt19937 gen(std::random_device{}());
        thread_local std::uniform_int_distribution<uint16_t> dist(0, 65535);

        do {

            sessionKey = dist(gen);


        } while (sessions.contains(sessionKey)); // 중복 체크

        sessions[sessionKey] = newSession;
    }
    newSession->Init(std::to_string(sessionKey),std::move(initInfo));
    newSession->RunAsync();

    return sessionKey;
}

std::shared_ptr<GameSession> SessionManager::getSessionById(const std::string &sessionId) {
    std::shared_lock lock(_sessionsLock);
    for (auto val: sessions | std::views::values) {
        if (val->getSessionId() == sessionId) {
            return val;
        }
    }
    return nullptr;
}


void SessionManager::cleanupSessions() {
    std::unique_lock lock(_sessionsLock);
    for (auto& session : sessions) {
        session.second->cleanUp();//delete
    }
    sessions.clear();
}

