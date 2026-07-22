//
// Created by white on 26. 7. 2..
//

#ifndef FPSPROJECTSERVER_ACTION_H
#define FPSPROJECTSERVER_ACTION_H
#include <vector>
#include <functional>
template<typename... Args>
class Action {
private:
    std::vector<std::function<void(Args...)>> listeners;

public:
    void AddListener(std::function<void(Args...)> listener) {
        listeners.push_back(listener);
    }

    void Invoke(Args... args) {
        for (const auto& listener : listeners) {
            if (listener) {
                listener(args...);
            }
        }
    }
};
#endif //FPSPROJECTSERVER_ACTION_H