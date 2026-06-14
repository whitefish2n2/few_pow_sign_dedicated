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
        static ObjectPool<T> instance;
        return instance;
    }

    // 풀에서 객체를 가져오거나 없으면 새로 만듦
    T* Acquire() {
        std::lock_guard<std::mutex> lock(mtx);
        if (pool.empty()) {
            return new T();
        }
        T* obj = pool.back();
        pool.pop_back();
        return obj;
    }

    // 객체를 다 쓰면 풀로 반환 (소멸시키지 않음!)
    void Release(T* obj) {
        std::lock_guard<std::mutex> lock(mtx);
        pool.push_back(obj);
    }
};
#endif //FPSPROJECTSERVER_OBJECTPOOL_H