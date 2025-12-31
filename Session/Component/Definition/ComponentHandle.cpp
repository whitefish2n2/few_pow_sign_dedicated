#include "../../SessionContext.h"
#include "ComponentManager.h"

template<typename T>
ComponentHandleBase ComponentHandle<T>::Clone() {
    static_assert(std::is_base_of_v<ComponentArgument, T>, "T MUST Driven By ComponentArgument To Access EntityId(ComponentHandle.cpp 6:)");
    auto newHandle = componentManagerInstance->CreateComponentAtPool<T>();
    T* dest = newHandle->operator->();
    T* src  = this->operator->();

    if (dest && src) {
        auto savedId = dest->entityId; // ID 백업
        *dest = *src;                  // T 타입 전체 데이터 복사
        dest->entityId = savedId;      // ID 복구
    }

    return newHandle;
}

template<typename T>
T *ComponentHandle<T>::operator->() {
    void* ptr = componentManagerInstance->GetRawPtr(this->typeId, this->entityId);
    return static_cast<T*>(ptr);
}



