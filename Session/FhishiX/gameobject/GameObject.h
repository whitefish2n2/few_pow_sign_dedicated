//
// Created by white on 25. 5. 20.
//

#ifndef OBJECT_H
#define OBJECT_H
#include <string>
#include <vector>
#include <memory>
#include "collider/Collider.h"
class GameSession;
class GameObjectArgument;
class GameObjectManager;

class GameObject {
    protected:
    long long targetId = -1;
    long generationId = -1;
    GameSession *gameSession;
    public:
    [[nodiscard]] long long GetId() const {return targetId;}
    [[nodiscard]] long GetGenerationId() const {return generationId;}
    GameObjectArgument* operator->() const;

    explicit GameObject(GameSession *owner);

    GameObject& operator=(const GameObject & target);
    bool operator==(const GameObject & target) const;
    explicit operator bool() const;
};
struct GameObjectHash {
    std::size_t operator()(const GameObject& k) const {
        return std::hash<long long>()(k.GetId()) ^ (std::hash<long>()(k.GetGenerationId()) << 1);
    }
};
#endif //OBJECT_H
