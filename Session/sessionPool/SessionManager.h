//
// Created by user on 25. 4. 27.
//

#ifndef SESSIONPOOL_H
#define SESSIONPOOL_H
#include <memory>
#include <mutex>
#include <vector>
#include "../GameSession.h"

class SessionManager {
public:
    std::vector<std::shared_ptr<GameSession>> sessions = std::vector<std::shared_ptr<GameSession>>();
    std::shared_ptr<GameSession> acquireSession();
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;
    static SessionManager& getInstance() {
        static SessionManager instance;
        return instance;
    }


    void addFinishedSession(std::shared_ptr<GameSession> session);

    void cleanupSessions();


private:
    SessionManager() = default;
    std::mutex mutex_;
};



#endif //SESSIONPOOL_H
