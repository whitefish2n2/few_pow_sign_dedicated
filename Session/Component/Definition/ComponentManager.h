//
// Created by white on 25. 12. 11.
//

#ifndef FPSPROJECTSERVER_COMPONENTMANAGER_H
#define FPSPROJECTSERVER_COMPONENTMANAGER_H
#include <iostream>
#include <memory>
#include <unordered_map>
#include "ComponentHandle.h"
#include "../../GameSession.h"
#include "../../SessionContext.h"
#include "../../../util/Log.h"
/*
 //fix
 *컴포넌트 패턴을 도입함. 컴포넌트는 해당 클래스의 각 타입별 Array에 저장되고, 소유권도 해당 클래스가 가짐.
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
    std::vector<size_t> indexArray;
    std::queue<size_t> freeIndices;
    static constexpr size_t PENDING_MASK = (size_t)1 << (sizeof(size_t) * 8 - 1);
    //유효하지 않은 인덱스
    static constexpr size_t INVALID_INDEX = (std::numeric_limits<size_t>::max)();
public:
    ComponentManager* componentManager;
    int poolPriority = 0;///Update 실행 우선순위
    virtual ~BasePool() = default;
    BasePool() = default;

    virtual void UpdateAll() = 0;
    ///엔티티 ID로 하여금 요소를 지우는 함수.
    virtual void DeleteComponent(ComponentEntityId entityId) = 0;
    ///유효한 엔티티 ID인지 확인하는 함수
    virtual bool ValidateHandle(ComponentEntityId entityId) {
        if (entityId >= indexArray.size()) return false;
        return indexArray[entityId] != INVALID_INDEX;
    }

    ///컴포넌트 생성/삭제 지연 처리 플러쉬 함수(매 Update 끝에 호출 요망)
    virtual void Flush() = 0;

    virtual ComponentArgument* GetArgument(ComponentEntityId id) = 0;
};
template<typename T>
class DrivenPool:public BasePool {
protected:
    std::vector<T> dataArray;
    std::vector<T> pendingAdds;
    ComponentEntityId nextId = 1;
public:
    DrivenPool(ComponentManager* manager){this->componentManager = manager;};
    auto begin() { return dataArray.begin(); }
    auto end() { return dataArray.end(); }
    auto cbegin() const { return dataArray.cbegin(); }
    auto cend() const { return dataArray.cend(); }

    void UpdateAll() override {
        size_t currentSize = dataArray.size();
        for (size_t i = 0; i < currentSize; ++i) {
            if (dataArray[i].isActive && !dataArray[i].willDead) {
                dataArray[i].Update();
            }
        }
    }
    ///새로운 Component 요소를 생성하는 함수, 해당 컴포넌트의 핸들을 반환한다.
    template<typename... Args>
    ComponentHandle<T> CreateComponent(Args&&... args );
    void DeleteComponent(const ComponentEntityId id) override {
        if (id >= indexArray.size()) return;

        size_t index = indexArray[id];
        if (index == INVALID_INDEX) return;

        if (index & PENDING_MASK) {
            size_t realIdx = index & ~PENDING_MASK;
            pendingAdds[realIdx].willDead = true;
        } else {
            dataArray[index].willDead = true;
        }
    }

    ComponentArgument* GetArgument(ComponentEntityId id) override {
        if (id >= indexArray.size()) return nullptr;

        // 2. 실제 인덱스 가져오기 (O(1), 캐시 친화적)
        size_t index = indexArray[id];
        if (index == INVALID_INDEX) return nullptr; // 삭제된 놈

        if (index & PENDING_MASK) {
            return &pendingAdds[index & ~PENDING_MASK];
        } else {
            return &dataArray[index];
        }
    }

    void Flush() override {
        // 1. 지연 삭제 처리: 메인 배열에서 죽은 놈들 Swap and Pop (인덱스 꼬임 방지를 위해 뒤에서부터)
        if (!dataArray.empty()) {
            for (size_t i = dataArray.size(); i-- > 0; ) {
                if (dataArray[i].willDead) {
                    ComponentEntityId deadId = dataArray[i].entityId;
                    size_t lastIndex = dataArray.size() - 1;

                    if (i != lastIndex) {
                        dataArray[i] = std::move(dataArray[lastIndex]);
                        ComponentEntityId movedId = dataArray[i].entityId;
                        indexArray[movedId] = i; // 마스크 없는 순수 인덱스로 덮어쓰기
                    }
                    dataArray.pop_back();
                    if(deadId < indexArray.size()) {
                        indexArray[deadId] = INVALID_INDEX;
                    }
                }
            }
        }

        // 2. 지연 생성 처리: 대기열(pendingAdds)의 데이터를 메인 배열로 병합
        for (size_t i = 0; i < pendingAdds.size(); ++i) {
            if (pendingAdds[i].willDead) {
                // 대기열에 들어가자마자 삭제된 놈은 맵에서 제거만 하고 스킵
                if(pendingAdds[i].entityId < indexArray.size()) {
                    indexArray[pendingAdds[i].entityId] = INVALID_INDEX;
                }
                continue;
            }

            size_t newIndex = dataArray.size();
            indexArray[pendingAdds[i].entityId] = newIndex; // 마스크를 벗긴 진짜 인덱스로 갱신
            dataArray.push_back(std::move(pendingAdds[i]));
        }

        // 3. 대기열 비우기
        pendingAdds.clear();
    }
};

template<typename T>
consteval int GetPoolPriority() {
    if constexpr (requires { T::UPDATE_PRIORITY; }) {
        return T::UPDATE_PRIORITY; // 선언되어 있으면 그 값 반환
    } else {
        return 0; // 선언 안 되어 있으면 기본값 0 반환
    }
}

class ComponentManager {
protected:
    std::vector<std::unique_ptr<BasePool>> pools;
    std::vector<BasePool*> updateOrder;
    bool isOrderDirty = false;
public:
    template<typename T>
    DrivenPool<T>* GetOrCreatePool() {
        size_t typeId = GetTypeId<T>();

        if (typeId >= pools.size()) {
            pools.resize(typeId + 1);
        }

        if (!pools[typeId]) {
            auto newPool = std::make_unique<DrivenPool<T>>(this);
            newPool->poolPriority = GetPoolPriority<T>();
            updateOrder.push_back(newPool.get());
            isOrderDirty = true;
            pools[typeId] = std::move(newPool);
        }

        return static_cast<DrivenPool<T>*>(pools[typeId].get());
    }
    GameSession *ownerSession;

    void UpdateComponents() {
        if (isOrderDirty) {
            std::sort(updateOrder.begin(), updateOrder.end(), [](BasePool* a, BasePool* b) {
                // 숫자가 클수록 먼저 Update
                return a->poolPriority > b->poolPriority;
            });
            isOrderDirty = false;
        }
        for (BasePool* pool : updateOrder) {
            pool->UpdateAll();
        }
        // 모든 Update가 끝나면 찌꺼기(지연 생성/삭제)들을 일괄 정리합니다.
        for (BasePool* pool : updateOrder) {
            pool->Flush();
        }
    }
    template<typename T>
    T* GetComponentFromPool(ComponentHandle<T>* handle) {
        size_t typeId = handle->getTypeId();
        if (pools.size()<=typeId || !pools[typeId] ) { return nullptr; };
        auto it = pools[typeId].get();
        return static_cast<T*>(it->GetArgument(handle->entityId));
    }

    template<typename T>
    void DeleteComponentFromPool(ComponentHandle<T>* handle) {
        size_t typeId = handle->getTypeId();
        if (pools.size()<=typeId || !pools[typeId]) { return; };
        auto it = pools[typeId].get();
        it->DeleteComponent(handle->entityId);
    }
    void DeleteComponentFromPoolById(size_t typeId, ComponentEntityId entityId) {
        if (pools.size() <= typeId || !pools[typeId]) { return; }
        pools[typeId]->DeleteComponent(entityId);
    }
    template<typename T, typename... Args>
    requires std::constructible_from<T, Args...>
    ComponentHandle<T> CreateComponentAtPool(Args&&... args) {
        std::cout<<"CreateComponentAtPool"<<std::endl;
        return GetOrCreatePool<T>()->CreateComponent(std::forward<Args>(args)...);
    }
    ///밖에서 생성된 컴포넌트(여기서 CreateComponentAtPool을 거치지 않고 생성된, 대표적으로 문자열->컴포넌트 파싱으로 생성된 컴포넌트)를 해당 ECS 인스턴스에 편입합니다.
    ///Component::MoveToManager()에서만 호출합니다.(T는 무조건 ComponentArgument여야 합니다.)
    template<typename T>
ComponentHandle<T> InsertOrphanageComponent(T* comp) {
        std::cout << "[InsertOrphan] 1. 진입" << std::endl;
        static_assert(std::is_base_of<ComponentArgument, T>::value, "T must inherit from ComponentArgument");

        std::cout << "[InsertOrphan] 2. CreateComponentAtPool 호출" << std::endl;
        ComponentHandle<T> newHandle = CreateComponentAtPool<T>();

        std::cout << "[InsertOrphan] 3. 핸들 생성 성공, PoolObj 가져오기" << std::endl;
        T* poolObj = GetComponentFromPool(&newHandle);
        ComponentEntityId newId = poolObj->entityId;

        std::cout << "[InsertOrphan] 4. Move 대입 연산자 실행 시도" << std::endl;
        if (comp) {
            *poolObj = std::move(*comp);
        }
        poolObj->entityId = newId;
        LOG_DEBUG("컴포넌트매니저에 GAMESESSION이 있을까요 없을까요???:::  ");
        LOG_DEBUG(((comp->gameSession) == nullptr? "응없어요" : "와있어요" ));
        if (poolObj->gameSession == nullptr) {
            poolObj->gameSession = this->ownerSession;
        }
        else {
            if (this->ownerSession == nullptr) {
                LOG_ERROR("ComponentManager: 아니 분명이 ptr 챙겨줬는데 왜터짐");
            }
        }
        std::cout << "[InsertOrphan] 5. 완료 및 반환" << std::endl;
        return newHandle;
    }
    ///outHandle에 생성된 객체의 ComponentHandleBase 핸들이 반환됩니다. orphan 객체는 호출 후 해제되니 접근할 수 없습니다.
    void RegisterOrphan(const std::unique_ptr<ComponentArgument> &orphan, ComponentHandleBase* outHandle) {
        if (!orphan) return;
        std::cout << "RegisterOrphan" << std::endl;
        orphan->MoveToManager(this, outHandle);
    }
    ///해당 타입의 특정 엔티티 id를 가진 객체의 ComponentArgument*(Raw PTR) 객체를 반환합니다.
    /// !! ALERT !! 해당 함수로 얻은 데이터를 캐싱하여 사용하지 마세요. 한 프레임정도는 버틸지 모르는데... 댕글링 포인터 위헙이 있습니다.
    ComponentArgument* GetRawPtr(size_t typeId, ComponentEntityId entity_id) {
        if (pools.size()<=typeId || !pools[typeId]) { return nullptr; };
        auto it = pools[typeId].get();
        return it->GetArgument(entity_id);
    }
};

//ComponentHandle 순환 참조 해결
template<typename T>
T* ComponentHandle<T>::operator->() {
    void* ptr = componentManager->GetRawPtr(this->typeId, this->entityId);
    if (ptr == nullptr) return nullptr;
    return static_cast<T*>(ptr);
}

template<typename T>
template<typename... Args>
ComponentHandle<T> DrivenPool<T>::CreateComponent(Args&&... args) {
    auto h = T(std::forward<Args>(args)...);
    ComponentEntityId entityId = nextId++;
    h.entityId = entityId;

    // 이제 ComponentManager가 완전히 정의된 상태이므로 에러가 나지 않습니다!
    h.gameSession = this->componentManager->ownerSession;

    if (entityId >= indexArray.size()) {
        indexArray.resize(entityId + 1, INVALID_INDEX);
    }
    // 메인 배열이 아닌 대기열(pendingAdds)에 밀어 넣습니다.
    size_t pendingIndex = pendingAdds.size();

    // 인덱스에 PENDING_MASK를 씌워서 배열에 저장합니다.
    indexArray[entityId] = pendingIndex | PENDING_MASK;
    pendingAdds.push_back(std::move(h));

    ComponentHandle<T> handle;
    handle.entityId = entityId;
    handle.typeId = GetTypeId<T>();
    handle.componentManager = componentManager;

    return handle;
}


template<typename T>
ComponentHandleBase ComponentHandle<T>::Clone() {
    if constexpr (std::is_abstract_v<T>) {
        return ComponentHandleBase::NULLPTR();
    }
    else {
        static_assert(std::is_base_of_v<ComponentArgument, T>, "T MUST Driven By ComponentArgument To Access EntityId(ComponentHandle.cpp 6:)");
        auto newHandle = componentManager->CreateComponentAtPool<T>();
        T* dest = newHandle.operator->();
        T* src  = this->operator->();

        if (dest && src) {
            auto savedId = dest->entityId; // ID 백업
            *dest = *src;                  // T 타입 전체 데이터 복사
            dest->entityId = savedId;      // ID 복구
        }

        return newHandle;
    }


}


#endif //FPSPROJECTSERVER_COMPONENTMANAGER_H