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
 //fix
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
public:
    virtual ~BasePool() = default;
    BasePool() = default;
    ///엔티티 ID로 하여금 요소를 지우는 함수.
    virtual void DeleteComponent(ComponentEntityId entityId) = 0;
    ///유효한 엔티티 ID인지 확인하는 함수
    virtual bool  ValidateHandle(ComponentEntityId entityId) {
        return indexArray.contains(entityId);
    }

    virtual ComponentArgument* GetArgument(ComponentEntityId id) = 0;
};
template<typename T>
class DrivenPool:public BasePool {
protected:
    std::vector<T> dataArray;
    ComponentEntityId nextId = 1;
public:
    DrivenPool()= default;
    ///새로운 Component 요소를 생성하는 함수, 해당 컴포넌트의 핸들을 반환한다.
    template<typename... Args>
    ComponentHandle<T> CreateComponent(Args&&... args ) {
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
            componentPool[typeId] = std::make_unique<DrivenPool<T>>();
        }
        return static_cast<DrivenPool<T>*>(componentPool[typeId].get());
    }
public:

    GameSession *ownerSession;

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
    requires std::constructible_from<T, Args...>
    ComponentHandle<T> CreateComponentAtPool(Args&&... args) {
        return GetOrCreatePool<T>()->CreateComponent(std::forward<Args>(args)...);
    }
    ///밖에서 생성된 컴포넌트(여기서 CreateComponentAtPool을 거치지 않고 생성된, 대표적으로 문자열->컴포넌트 파싱으로 생성된 컴포넌트)를 해당 ECS 인스턴스에 편입합니다.
    ///Component::MoveToManager()에서만 호출합니다.(T는 무조건 ComponentArgument여야 합니다.)
    template<typename T>
    ComponentHandle<T> InsertOrphanageComponent(T* comp) {
        static_assert(std::is_base_of<ComponentArgument, T>::value,
                  "T must inherit from ComponentArgument!(ComponentManager,122)");
        ComponentHandle<T> newHandle = CreateComponentAtPool<T>();

        //  데이터 이동
        T* poolObj = GetComponentFromPool(&newHandle);
        ComponentEntityId newId = poolObj->entityId;
        if (comp) {
            *poolObj = std::move(*comp);
        }


        poolObj->entityId = newId;


        return newHandle;
    }
    ///outHandle에 생성된 객체의 ComponentHandleBase 핸들이 반환됩니다. orphan 객체는 호출 후 해제되니 접근할 수 없습니다.
    void RegisterOrphan(const std::unique_ptr<ComponentArgument> &orphan, ComponentHandleBase* outHandle) {
        if (!orphan) return;
        orphan->MoveToManager(this, outHandle);
    }
    ///해당 타입의 특정 엔티티 id를 가진 객체의 ComponentArgument*(Raw PTR) 객체를 반환합니다.
    /// !! ALERT !! 해당 함수로 얻은 데이터를 캐싱하여 사용하지 마세요. 댕글링 포인터 위헙이 있습니다.
    ComponentArgument* GetRawPtr(size_t type_id, ComponentEntityId entity_id) {
        if (!componentPool.contains(type_id)) return nullptr;
        return componentPool[type_id]->GetArgument(entity_id);
    }
};

//ComponentHandle 순환 참조 해결
template<typename T>
T* ComponentHandle<T>::operator->() {
    void* ptr = componentManagerInstance->GetRawPtr(this->typeId, this->entityId);
    return static_cast<T*>(ptr);
}
template<typename T>
ComponentHandleBase ComponentHandle<T>::Clone() {
    if constexpr (std::is_abstract_v<T>) {
        return ComponentHandleBase::NULLPTR();
    }
    else {
        static_assert(std::is_base_of_v<ComponentArgument, T>, "T MUST Driven By ComponentArgument To Access EntityId(ComponentHandle.cpp 6:)");
        auto newHandle = componentManagerInstance->CreateComponentAtPool<T>();
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