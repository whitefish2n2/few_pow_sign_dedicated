//
// Created by user on 25. 4. 27.
//

#ifndef SESSIONPOOL_H
#define SESSIONPOOL_H
#include <memory>
#include <mutex>
#include <queue>
#include <vector>
#include "../GameSession.h"

class SessionManager {
public:
    std::queue<std::shared_ptr<GameSession>> EventQueue;
    std::unordered_map<std::uint16_t, std::shared_ptr<GameSession>> sessions;
    std::shared_ptr<GameSession> acquireSession();
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;
    static SessionManager& getInstance() {
        static SessionManager instance;
        return instance;
    }

    uint16_t makeNewSession(GameSetupBoddari initInfo);


    void addFinishedSession(std::shared_ptr<GameSession> session);

    void cleanupSessions();


private:
    SessionManager() = default;
    std::mutex mutex_;
    uint16_t sessionKeyRoundRobin = 0;
};



#endif //SESSIONPOOL_H
