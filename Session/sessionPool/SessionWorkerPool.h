#ifndef SESSIONWORKERPOOL_H
#define SESSIONWORKERPOOL_H
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "../GameSession.h"

class SessionWorkerPool {
public:
    SessionWorkerPool(const SessionWorkerPool&) = delete;
    SessionWorkerPool& operator=(const SessionWorkerPool&) = delete;
    static SessionWorkerPool& getInstance() {
        static SessionWorkerPool instance;
        return instance;
    }

    // workerCount <= 0 이면 hardware_concurrency() 기반으로 자동 결정
    void Start(int workerCount = 0);
    void AssignSession(const std::shared_ptr<GameSession>& session);
    void RemoveSession(const std::shared_ptr<GameSession>& session);
    size_t GetWorkerCount() const { return workers.size(); }

private:
    SessionWorkerPool() = default;

    struct Worker {
        std::thread thread;
        std::mutex sessionsLock;
        std::vector<std::shared_ptr<GameSession>> sessions;
    };

    std::vector<std::unique_ptr<Worker>> workers;
    std::atomic<size_t> nextWorkerIndex{0};

    void WorkerLoop(Worker* worker);
};
#endif //SESSIONWORKERPOOL_H
