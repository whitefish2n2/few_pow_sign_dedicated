#pragma once

#include <string>
#include <map>
#include <functional>
#include <iostream>

#include "ComponentArgument.h"

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

    ///타입 이름(Class명과 일치)을 넣고 해당 타입의 ParseFromString에서 파싱할 수 있는 String을 arg에 입력하면 object에 Attach합니다. 성공 결과를 boolean으로 반환합니다.
    bool Create(const std::string& typeName, const GameObject &object, const std::string &arg="") {
        try {
            auto it = creators.find(typeName);
            if (it != creators.end()) {
                auto comp =  it->second(object);
                if (comp == nullptr) return false;
                comp->ParseFromString(arg);
                return true;
            }
            return false;
        }
        catch (std::exception e) {
            std::cout<<e.what()<<std::endl;
            std::cout<<typeName<<"파싱 실패"<<std::endl;
            return false;
        }

    }
    ~ComponentFactory() = default;
private:
    // ComponentFactory() = default; // 이 부분을 아래와 같이 변경
    ComponentFactory() = default;

    // 소멸자도 static 인스턴스 해제 시 접근 문제가 생길 수 있으므로
    // private에 둘 경우 주의가 필요하나, 내부 정적 변수라 보통은 괜찮습니다.


    std::map<std::string, CreatorFunc> creators;
};

///String 컴포넌트-> 컴포넌트 객체로 변환을 위한 매크로
///ComponentArgument 구현 객체에 REGISTER_COMPONENT(TYPE)형태로 사용하면 타입 이름->타입 매핑이 생성됩니다.
#define REGISTER_COMPONENT(TYPE) \
static struct Register##TYPE { \
Register##TYPE() { \
ComponentFactory::Instance().Register(#TYPE, [](const GameObject& obj) -> ComponentArgument* { \
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
TYPE* rawPtr = handle.operator->(); \
\
/* 4. ComponentArgument* 로 업캐스팅되어 반환 (파싱용) */ \
return rawPtr; \
}); \
} \
} global_register_##TYPE;