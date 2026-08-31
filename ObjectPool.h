//
// Created by white on 26. 5. 27..
//

#ifndef FPSPROJECTSERVER_OBJECTPOOL_H
#define FPSPROJECTSERVER_OBJECTPOOL_H
#include <mutex>
#include <vector>
#include <memory>

template <typename T>
class ObjectPool {
private:
    std::vector<T*> pool;
    std::mutex mtx;

public:
    static ObjectPool<T>& GetInstance() {
        static thread_local ObjectPool<T> instance;
        return instance;
    }

    T* Acquire() {
        if (pool.empty()) {
            return new T();
        }
        T* obj = pool.back();
        pool.pop_back();
        return obj;
    }
    void Release(T* obj) {
        pool.push_back(obj);
    }
};
#endif //FPSPROJECTSERVER_OBJECTPOOL_H