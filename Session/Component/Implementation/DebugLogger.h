//
// Created by white on 26. 3. 13..
//

#ifndef FPSPROJECTSERVER_DEBUGLOGGER_H
#define FPSPROJECTSERVER_DEBUGLOGGER_H
#include <iostream>
#include <ostream>

#include "../Definition/Component.h"
#include "../Definition/ComponentArgument.h"

class  DebugLogger final:public Component<DebugLogger> {
    public:
    void FixedUpdate() override {
        std::cout << "Debug logger updated" << std::endl;
    }
    ;
};


#endif //FPSPROJECTSERVER_DEBUGLOGGER_H