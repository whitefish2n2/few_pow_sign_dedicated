//
// Created by white on 26. 1. 6..
//

#ifndef FPSPROJECTSERVER_COMPONENTFACTORY_H
#define FPSPROJECTSERVER_COMPONENTFACTORY_H
#pragma once
#include <string>
#include <map>
#include <functional>
#include <iostream>
#include <utility>

#include "ComponentArgument.h"

class ComponentFactory {
public:
    // GameObject를 받아서 그 오브젝트에 컴포넌트를 붙이는 함수 타입
    using CreatorFunc = std::function<ComponentArgument*(GameObject)>;

    static ComponentFactory& Instance() {
        thread_local ComponentFactory instance;
        return instance;
    }

    void Register(const std::string& name, CreatorFunc creator) {
        creators[name] = std::move(creator);
    }

    // 호출 시 GameObject를 넘겨줍니다.
    ComponentArgument* CreateAndAttach(const std::string& name, GameObject obj) {
        if (creators.find(name) != creators.end()) {
            return creators[name](obj);
        }
        return nullptr;
    }

private:
    std::map<std::string, CreatorFunc> creators;
};
#define REGISTER_COMPONENT(TYPE) \
    static struct Register##TYPE { \
        Register##TYPE() { \
            ComponentFactory::Instance().Register(#TYPE, [](GameObject obj) -> Component* { \
                /* 1. 기존 시스템(매니저)을 통해 생성하고 핸들을 받습니다 */ \
                /* ComponentHandle<BoxCollider> handle = ... */ \
                auto handle = obj->AddComponent<TYPE>(); \
                \
                /* 2. 유효성 체크 (님의 NULLPTR 구현을 보니 entityId -1이 null이군요) */ \
                if (handle.entityId == static_cast<ComponentEntityId>(-1)) { \
                    return nullptr; \
                } \
                \
                /* 3. operator->()를 호출하여 T* (Raw Pointer)를 꺼냅니다 */ \
                /* 이때 내부적으로 componentManagerInstance->GetRawPtr()이 실행됩니다 */ \
                (TYPE)* rawPtr = handle.operator->(); \
                \
                /* 4. Component* 로 업캐스팅되어 반환 (파싱용) */ \
                return rawPtr; \
}); \
} \
} global_register_##TYPE;