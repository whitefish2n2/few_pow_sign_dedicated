//
// Created by white on 26. 3. 26..
//

#ifndef FPSPROJECTSERVER_LOG_H
#define FPSPROJECTSERVER_LOG_H

#pragma once
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <sstream>
#include <atomic>

enum class LogLevel { DEBUG, INFO, WARN, ERR };

class Logger {
public:
    static Logger& Get() {
        static Logger instance;
        return instance;
    }

    void SetLevel(LogLevel level) { minLevel_ = level; }

    void Log(LogLevel level, const std::string& msg,
             const char* file, int line)
    {
        if (level < minLevel_) return;

        std::ostringstream oss;
        oss << "[" << LevelStr(level) << "] "
            << Filename(file) << ":" << line << "  " << msg;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(oss.str());
        }
        cv_.notify_one();
    }

    ~Logger() {
        running_ = false;
        cv_.notify_one();
        worker_.join();
    }

private:
    Logger() : worker_([this] { Work(); }) {}
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void Work() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !queue_.empty() || !running_; });

            while (!queue_.empty()) {
                std::cout << queue_.front() << "\n";
                queue_.pop();
            }

            if (!running_) break;
        }
    }

    static const char* LevelStr(LogLevel l) {
        switch (l) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO ";
            case LogLevel::WARN:  return "WARN ";
            case LogLevel::ERR:   return "ERROR";
        }
        return "?????";
    }

    // "src/foo/bar.cpp" → "bar.cpp"
    static const char* Filename(const char* path) {
        const char* p = path;
        const char* last = path;
        while (*p) {
            if (*p == '/' || *p == '\\') last = p + 1;
            ++p;
        }
        return last;
    }

    std::queue<std::string> queue_;
    std::mutex              mutex_;
    std::condition_variable cv_;
    std::thread             worker_;
    std::atomic<bool>       running_{ true };
    std::atomic<LogLevel>   minLevel_{ LogLevel::DEBUG };
};

// ── 매크로 ──────────────────────────────────────────────
#define LOG_DEBUG(msg) //Logger::Get().Log(LogLevel::DEBUG, msg, __FILE__, __LINE__)
#define LOG_INFO(msg)  //Logger::Get().Log(LogLevel::INFO,  msg, __FILE__, __LINE__)
#define LOG_WARN(msg)  //Logger::Get().Log(LogLevel::WARN,  msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) //Logger::Get().Log(LogLevel::ERR,   msg, __FILE__, __LINE__)
#endif //FPSPROJECTSERVER_LOG_H