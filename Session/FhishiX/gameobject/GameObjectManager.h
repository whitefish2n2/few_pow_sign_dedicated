//
// Created by white on 25. 12. 2..
//

#ifndef FPSPROJECTSERVER_GAMEOBJECTMANAGER_H
#define FPSPROJECTSERVER_GAMEOBJECTMANAGER_H
#include <unordered_map>

#include "GameObjectArgument.h"
#include "GameObject.h"
#include "../../GameSession.h"

class GameSession;

class GameObjectManager {
    protected:
    GameSession* ownerSession;
    std::unordered_map<GameObject, GameObjectArgument,GameObjectHash> gameObjects;
    struct EntitySlot {
        uint32_t generation;
        bool isActive = false;
        GameObjectArgument argument;
    };
    std::vector<GameObjectArgument> objects;

    ///삭제된 GameObject들의 인덱스 위치들
    std::queue<uint32_t>freeIndices;
    public:
    GameObject CreateGameObject() {
        if (freeIndices.empty()) {
            auto obj = GameObjectArgument(ownerSession);
            objects.push_back(obj);

        }
    }
    GameObjectManager(GameSession* session) {
        ownerSession = session;
    };
    GameObjectArgument* GetGameObject(const GameObject* ref) {
        if (gameObjects.contains(*ref)) {
            return &gameObjects[*ref];
        }
        else {
            return nullptr;
        }
    }
};
#endif //FPSPROJECTSERVER_GAMEOBJECTMANAGER_H