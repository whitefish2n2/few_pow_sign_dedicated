//
// Created by white on 26. 1. 26..
//
#include "ComponentFactory.h"

#include "../../../util/util.h"

ComponentHandleBase ComponentFactory::Create(const std::string &typeName, const GameObject &parent,
                                             const std::string &arg, ComponentManager *componentManager) {

    std::cout << "  [Create] 1. 진입 (TypeName: " << typeName << ")" << std::endl;

    try {
        auto it = creators.find(typeName);
        if (it != creators.end()) {
            std::unique_ptr<ComponentArgument> comp(it->second(parent));
            LOG_INFO("parent 타겟 아이디 ::" + parent.targetId);
            comp->gameObject = parent;
            comp->ParseFromString(arg);
            comp->SetOwner(parent);
            comp->gameSession = parent->gameSession;
            Log("GAMESESSION이 있을까요 없을까요???:::  ");
            Log(((comp->gameSession) == nullptr? "응없어요" : "와있어요" ));
            ComponentHandleBase base(comp.get()->typeId, comp.get()->entityId,componentManager);
            componentManager->RegisterOrphan(comp, &base);
            parent->AttachComponentBase(base);

            return base;
        }else {
            LOG_ERROR("팩토리에 등록되지 않은 컴포넌트입니다!!! 오타 확인 요망: " + typeName);
        }
        return ComponentHandleBase::NULLPTR();
    }
    catch (const std::exception& e) {
        return ComponentHandleBase::NULLPTR();
    }
    catch (...) {
        return ComponentHandleBase::NULLPTR();
    }
}
