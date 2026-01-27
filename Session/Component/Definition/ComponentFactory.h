#pragma once

#include <string>
#include <map>
#include <functional>
#include <iostream>

#include "ComponentArgument.h"
#include "ComponentHandleBase.h"
#include "../../FhishiX/gameobject/GameObjectArgument.h"

class ComponentFactory {
public:
    using CreatorFunc = std::function<ComponentArgument*(GameObject)>;

    static ComponentFactory& Instance() {
        static ComponentFactory instance;
        return instance;
    }

    ComponentFactory(const ComponentFactory&) = delete;
    ComponentFactory& operator=(const ComponentFactory&) = delete;

    void Register(const std::string& name, CreatorFunc creator) {
        creators[name] = std::move(creator);
    }

    ///타입 이름(Class명과 일치)을 넣고 해당 타입의 ParseFromString에서 파싱할 수 있는 String을 arg에 입력하면 object에 Attach합니다. 실패시 ComponentHandleBase::NULLPTR을 반환합니다.
    ComponentHandleBase Create(const std::string& typeName, const GameObject &object, const std::string &arg, ComponentManager *componentManager);

    ~ComponentFactory() = default;
private:
    ComponentFactory() = default;



    std::map<std::string, CreatorFunc> creators;
};

///String 컴포넌트-> 컴포넌트 객체로 변환을 위한 매크로
///ComponentArgument 구현 객체에 REGISTER_COMPONENT(TYPE)형태로 사용하면 타입 이름->타입 매핑이 생성됩니다.
#define REGISTER_COMPONENT(TYPE) \
static struct Register##TYPE { \
Register##TYPE() { \
ComponentFactory::Instance().Register(#TYPE, [](const GameObject& obj) -> ComponentArgument* { \
auto handle = obj->AddComponent<TYPE>(); \
\
if (handle.entityId == static_cast<ComponentEntityId>(-1)) { \
return nullptr; \
} \
\
TYPE* rawPtr = handle.operator->(); \
\
return rawPtr; \
}); \
} \
} global_register_##TYPE;