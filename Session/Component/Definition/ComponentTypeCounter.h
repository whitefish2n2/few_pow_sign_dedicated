//
// Created by white on 25. 12. 16..
//

#ifndef FPSPROJECTSERVER_COMPONENTTYPECOUNTER_H
#define FPSPROJECTSERVER_COMPONENTTYPECOUNTER_H
#include <atomic>

///컴포넌트들의 타입에 대해 Type Id를 매겨주는 클래스, 함수
class ComponentTypeCounter {
    protected:
        inline static std::atomic<size_t> typeIdCounter = 0;
    public:
        static size_t GetNextTypeId() {return typeIdCounter++;}
};


template<class T>
size_t GetTypeId() {
    static const size_t type = ComponentTypeCounter::GetNextTypeId();
    return type;
}
#endif //FPSPROJECTSERVER_COMPONENTTYPECOUNTER_H