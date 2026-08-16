//
// Created by white on 26. 1. 26..
//

#ifndef FPSPROJECTSERVER_COMPONENTCONSTRUCTOR_H
#define FPSPROJECTSERVER_COMPONENTCONSTRUCTOR_H
#include <iostream>
#include <string>

#include "../../../FhishiX/gameobject/GameObjectArgument.h"


class ComponentConstructor {
    public:
    std::string ComponentName;
    std::string Argument;
    ///컴포넌트를 생성하여 인수로 전달된 오브젝트에 Attach합니다.
    void ConstructAndAttachTo(const GameObject &gameObject) const {
        //std::cout<<"AddComponentFromString"<< std::endl;
        gameObject->AddComponentFromString(ComponentName,Argument);
    };
};


#endif //FPSPROJECTSERVER_COMPONENTCONSTRUCTOR_H