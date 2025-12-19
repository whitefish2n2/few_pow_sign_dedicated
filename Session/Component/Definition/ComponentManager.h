//
// Created by white on 25. 12. 11.
//

#ifndef FPSPROJECTSERVER_COMPONENTMANAGER_H
#define FPSPROJECTSERVER_COMPONENTMANAGER_H
#include <memory>
#include <unordered_map>
#include "ComponentHandle.h"
#include "../../SessionContext.h"
/*
 *ECS 패턴을 도입함. 컴포넌트는 해당 클래스의 각 타입별 Array에 저장되고, 소유권도 해당 클래스가 가짐.
 *외부에서 사용되는 Component는 해당 클래스에 접근하는 핸들의 역할을 함.
 *Componant를 받으면 컴포넌트의 엔티티 아이디(핸들에 저장)->맵에 저장된 인덱스로 변환,
 *해당 타입을 저장하는 Array의 해당 인덱스에 있는 엔티티를 가져오게 됨.
 *Component가 삭제될 때에는 엔티티 아이디->맵 인덱스 변환 맵에서 삭제시키고, 해당 Component가 조회될 시에는 nullptr을 반환함.
 *Component가 삭제되어도 SOA(Structure Of Arrays)패턴을 유지하기 위해  swap and pop 방식을 채용, 해당 타입 Array의 맨 끝에 있는 요소를 삭제된 Component가 있던 빈칸에 삽입 후
 *상응하는 맵->인덱스 변환 어레이의 해당 오브젝트 인덱스 값을 변경하여 핸들 유효성을 유지시킴.
 *해당 방식은 캐시 히트율을 높여주며, 단편화 현상도 해결함.
 */

class BasePool {
    protected:
    std::unordered_map<ComponentEntityId, size_t> indexArray;
    GameSession * session;
public:
    virtual ~BasePool() = default;
    BasePool(GameSession * session):session(session){};
    ///엔티티 ID로 하여금 요소를 지우는 함수.
    virtual void DeleteComponent(ComponentEntityId entityId) = 0;
    ///유효한 엔티티 ID인지 확인하는 함수
    virtual bool  ValidateHandle(ComponentEntityId entityId) {
        return indexArray.find(entityId) != indexArray.end();
    }
    virtual ComponentArgument* GetArgument(ComponentEntityId id) = 0;
};
template<typename T>
class DrivenPool:public BasePool {
protected:
    std::vector<T> dataArray;
    ComponentEntityId nextId = 1;
public:
    DrivenPool(GameSession * session):BasePool(session){};
    ///새로운 Component 요소를 생성하는 함수, 해당 컴포넌트의 핸들을 반환한다.
    template<typename... Args>
    ComponentHandle<T> CreateComponent(GameSession* session, Args&&... args ) {
        auto h = T(std::forward<Args>(args)...);
        ComponentEntityId entityId = nextId++;
        h.entityId = entityId;
        size_t index = dataArray.size();
        indexArray[entityId] = index;
        dataArray.push_back(std::move(h));

        ComponentHandle<T> handle;
        handle.entityId = entityId;

        return handle;
    }
    void DeleteComponent(const ComponentEntityId entityIdToDelete) override {
        auto it = indexArray.find(entityIdToDelete);
        if (it==indexArray.end()) return;
        uint64_t index = it->second;
        uint64_t lastIndex = indexArray.size() - 1;
        if (index!=lastIndex) {
            dataArray[index] = std::move(dataArray[lastIndex]);
            ComponentEntityId entityId = dataArray[index].entityId;
            indexArray[entityId] = index;
        }
        dataArray.pop_back();
        indexArray.erase(entityIdToDelete);
    }
    //override
    ComponentArgument* GetArgument(ComponentEntityId id) override {
        auto it = indexArray.find(id);
        if (it==indexArray.end()) return nullptr;
        return &dataArray[it->second];
    }
};

class ComponentManager {
protected:
    std::unordered_map<size_t, std::unique_ptr<BasePool>> componentPool;
    template<typename T>
    DrivenPool<T>* GetOrCreatePool() {
        size_t typeId = ComponentHandle<T>::getTypeId();
        if (!componentPool.contains(typeId)) {
            componentPool[typeId] = std::make_unique<DrivenPool<T>>(gameSessionInstance);
        }
        return static_cast<DrivenPool<T>*>(componentPool[typeId].get());
    }
public:

    template<typename T>
    T* GetComponentFromPool(ComponentHandle<T>* handle) {
        size_t typeId = handle->getTypeId();
        const auto it = componentPool.find(typeId);
        if (it == componentPool.end()) return nullptr;
        return static_cast<T*>(it->second->GetArgument(handle->entityId));
    }

    template<typename T>
    void DeleteComponentFromPool(ComponentHandle<T>* handle) {
        size_t typeId = handle->getTypeId();
        auto it = componentPool.find(typeId);
        if (it == componentPool.end()) return;
        it->second->DeleteComponent(handle->entityId);
    }

    template<typename T, typename... Args>
    ComponentHandle<T> CreateComponentAtPool(Args&&... args) {
        return GetOrCreatePool<T>()->CreateComponent(gameSessionInstance, std::forward<Args>(args)...);
    }
};


#endif //FPSPROJECTSERVER_COMPONENTMANAGER_H