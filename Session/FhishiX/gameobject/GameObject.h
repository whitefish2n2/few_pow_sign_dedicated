//
// Created by white on 25. 5. 20.
//

#ifndef OBJECT_H
#define OBJECT_H
#include "EntityTypes.h"
class GameObjectArgument;
class GameSession;
class GameObjectManager;

class GameObject {
    protected:
    GameObjectId targetId = -1;
    GameObjectGenerationId generationId = -1;
    GameSession *session = nullptr;
    public:
    [[nodiscard]] GameObjectId GetId() const {return targetId;}
    [[nodiscard]] GameObjectGenerationId GetGenerationId() const {return generationId;}
    GameObjectArgument* operator->() const;

    GameObject(GameObjectId targetId, GameObjectGenerationId gen);
    GameObject()=default;
    GameObject& operator=(const GameObject & target);
    GameObject(const GameObject & other):targetId(other.targetId),generationId(other.generationId){};
    bool operator==(const GameObject & target) const;
    explicit operator bool() const;

    static GameObject NullPTR() {
        return GameObject(-1,-1);
    }
    static bool IsNull(const GameObject &target);
};
#endif //OBJECT_H
