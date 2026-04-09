#include "GameObjectArgument.h"

#include "../../Component/Definition/ComponentFactory.h"

//
// Created by white on 25. 10. 28.
//

GameObject GameObjectArgument::MakeHandle() const {
    if (id == -1) LOG_INFO("NULLPTR 게임오브젝트 객체에 대한 핸들 생성");
    return {id,generationId, gameSession};
}

void GameObjectArgument::AddComponentFromString(const std::string &typeName, const std::string &arg) const {
    std::cout<<"MakeHandle"<< std::endl;
    auto v = ComponentFactory::Instance().Create(typeName,MakeHandle(),arg,gameSession->componentManager.get());
}