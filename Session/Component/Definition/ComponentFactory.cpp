//
// Created by white on 26. 1. 26..
//
#include "ComponentFactory.h"

ComponentHandleBase ComponentFactory::Create(const std::string &typeName, const GameObject &object,
    const std::string &arg, ComponentManager *componentManager) {
    try {
        auto it = creators.find(typeName);
        if (it != creators.end()) {
            auto comp =  it->second(object);
            comp->SetOwner(object);
            comp->ParseFromString(arg);
            ComponentHandleBase base;
            componentManager->RegisterOrphan(comp,&base);
            return base;
        }
        return ComponentHandleBase::NULLPTR();
    }
    catch (std::exception* e) {
        std::cout<<e->what()<<std::endl;
        std::cout<<typeName<<"파싱 실패"<<std::endl;
        return ComponentHandleBase::NULLPTR();
    }

}
