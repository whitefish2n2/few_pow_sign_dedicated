#include "SessionWorkerPool.h"

#include <algorithm>
#include <chrono>
#include <iostream>

#include "../../ServerStatics.h"

void SessionWorkerPool::Start(int workerCount) {
    if (workerCount <= 0) {
        unsigned int hw = std::thread::hardware_concurrency();
        workerCount = hw > 3 ? static_cast<int>(hw) - 3 : 1;
        if (workerCount <= 0) workerCount = 4;
    }
    std::cout << "[SessionWorkerPool] starting " << workerCount << " workers" << std::endl;

    for (int i = 0; i < workerCount; ++i) {
        auto worker = std::make_unique<Worker>();
        Worker* raw = worker.get();
        worker->thread = std::thread([this, raw]() { WorkerLoop(raw); });
        workers.push_back(std::move(worker));
    }
}

void SessionWorkerPool::AssignSession(const std::shared_ptr<GameSession>& session) {
    if (workers.empty()) return;
    size_t idx = nextWorkerIndex.fetch_add(1, std::memory_order_relaxed) % workers.size();
    auto& worker = workers[idx];
    std::lock_guard<std::mutex> lock(worker->sessionsLock);
    worker->sessions.push_back(session);
}

void SessionWorkerPool::RemoveSession(const std::shared_ptr<GameSession>& session) {
    for (auto& worker : workers) {
        std::lock_guard<std::mutex> lock(worker->sessionsLock);
        auto& list = worker->sessions;
        list.erase(std::remove(list.begin(), list.end(), session), list.end());
    }
}

void SessionWorkerPool::WorkerLoop(Worker* worker) {
    constexpr auto PASS_BUDGET = std::chrono::milliseconds(10);
    constexpr auto MIN_SLEEP = std::chrono::milliseconds(2);

    while (isRunning.load()) {
        auto passStart = std::chrono::steady_clock::now();

        std::vector<std::shared_ptr<GameSession>> snapshot;
        {
            std::lock_guard<std::mutex> lock(worker->sessionsLock);
            snapshot = worker->sessions;
        }
        for (auto& session : snapshot) {
            session->TryTick();
        }

        auto remaining = PASS_BUDGET - (std::chrono::steady_clock::now() - passStart);
        if (remaining > MIN_SLEEP) {
            std::this_thread::sleep_for(remaining - std::chrono::milliseconds(1));
        }
    }
}
