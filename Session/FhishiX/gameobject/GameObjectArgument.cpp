#include "GameObjectArgument.h"

#include "../../Component/Definition/ComponentFactory.h"

//
// Created by white on 25. 10. 28.
//

GameObject GameObjectArgument::MakeHandle() const {
    return {id,generationId};
}

void GameObjectArgument::AddComponentFromString(const std::string &typeName, const std::string &arg) const {
    ComponentFactory::Instance().Create(typeName,MakeHandle(),arg,gameSession->componentManager);
}





