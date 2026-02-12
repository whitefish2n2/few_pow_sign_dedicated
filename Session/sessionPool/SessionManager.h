//
// Created by user on 25. 4. 27.
//

#ifndef SESSIONPOOL_H
#define SESSIONPOOL_H
#include <memory>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <vector>
#include "../GameSession.h"

class SessionManager {
public:
    std::shared_mutex _sessionsLock;
    std::unordered_map<std::uint16_t, std::shared_ptr<GameSession>> sessions;
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;
    static SessionManager& getInstance() {
        static SessionManager instance;
        return instance;
    }

    std::vector<std::weak_ptr<GameSession>> getSessionListWeak();

    uint16_t makeNewSession(GameSetupBoddari initInfo);

    std::shared_ptr<GameSession> getSessionById(const std::string &sessionId);


    void addFinishedSession(std::shared_ptr<GameSession> session);

    void cleanupSessions();


private:
    SessionManager() = default;
    uint16_t sessionKeyRoundRobin = 0;
};



#endif //SESSIONPOOL_H
