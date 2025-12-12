//
// Created by white on 25. 12. 11..
//

#ifndef FPSPROJECTSERVER_COMPONENTHANDLE_H
#define FPSPROJECTSERVER_COMPONENTHANDLE_H
#include <typeinfo>

#include "EntityTypes.h"
#include "../../GameSession.h"

class GameSession;
class ComponentArgument;
template<typename T>
class ComponentHandle {
    public:
    ComponentEntityId entityId;
    GameSession *session;
    static size_t getTypeId() {
        static const size_t type = ComponentManager::ComponentTypeCounter::GetNextTypeId();
         return type;
    }
    ComponentArgument*  operator->();
};
#endif //FPSPROJECTSERVER_COMPONENTHANDLE_H
