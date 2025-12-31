#include "GameObjectArgument.h"

//
// Created by white on 25. 10. 28.
//

GameObject GameObjectArgument::MakeHandle() const {
    return {id,generationId};
}


template<typename  T, typename... Args>
requires std::constructible_from<T, Args...>
ComponentHandle<T> GameObjectArgument::AddComponent(Args &&... args) {
    static_assert(std::is_base_of_v<ComponentArgument, T>, "T must derive from Component");
    ComponentHandle<T> handleT = componentManagerInstance->CreateComponentAtPool<T>(std::forward<Args>(args)...);
    handleT->SetOwner(MakeHandle());
    ComponentHandleBase componentBase = handleT;
    componentBase.typeId = handleT.getTypeId();
    components.push_back(std::move(componentBase));
    return handleT;
}


