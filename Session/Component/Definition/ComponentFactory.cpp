//
// Created by white on 26. 1. 26..
//
#include "ComponentFactory.h"

ComponentHandleBase ComponentFactory::Create(const std::string &typeName, const GameObject &parent,
    const std::string &arg, ComponentManager *componentManager) {

    std::cout << "  [Create] 1. 진입 (TypeName: " << typeName << ")" << std::endl;

    try {
        auto it = creators.find(typeName);
        if (it != creators.end()) {
            std::cout << "  [Create] 2. Creator 람다 호출" << std::endl;
            std::unique_ptr<ComponentArgument> comp(it->second(parent));

            std::cout << "  [Create] 3. SetOwner 호출" << std::endl;
            comp->SetOwner(parent);

            std::cout << "  [Create] 4. ParseFromString 호출" << std::endl;
            comp->ParseFromString(arg); // 💥 CalculateAABB() 등에서 터지면 여기서 멈춤!

            std::cout << "  [Create] 5. RegisterOrphan 진입 준비" << std::endl;
            ComponentHandleBase base(comp.get()->typeId, comp.get()->entityId,componentManager);
            componentManager->RegisterOrphan(comp, &base);

            std::cout << "  [Create] 6. 생성 완료!" << std::endl;
            return base;
        }
        std::cout << "  [Create] 💥 등록되지 않은 컴포넌트입니다." << std::endl;
        return ComponentHandleBase::NULLPTR();
    }
    // 💡 [매우 중요] std::exception* 가 아니라 & 로 잡아야 C++ 표준 예외를 잡을 수 있습니다!
    catch (const std::exception& e) {
        std::cout << "  [Create] 💥 파싱 중 C++ 예외 발생: " << e.what() << std::endl;
        return ComponentHandleBase::NULLPTR();
    }
    catch (...) {
        std::cout << "  [Create] 💥 알 수 없는 치명적 예외 발생!" << std::endl;
        return ComponentHandleBase::NULLPTR();
    }
}
