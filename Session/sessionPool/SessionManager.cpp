//
// Created by user on 25. 4. 27.
//

#include "SessionManager.h"

#include <mutex>


    void SessionManager::addFinishedSession(std::shared_ptr<GameSession> session) {
        std::lock_guard<std::mutex> lock(mutex_);
        sessions.push_back(std::move(session));
    }

    void SessionManager::cleanupSessions() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& session : sessions) {
            session->cleanUp(); // 리소스 해제용 함수 호출
        }
        sessions.clear();
    }
std::shared_ptr<GameSession> SessionManager::acquireSession() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!sessions.empty()) {
        auto session = sessions.back();
        sessions.pop_back();
        session->reset();
        return session;
    } else {
        return std::make_shared<GameSession>();
    }
}

