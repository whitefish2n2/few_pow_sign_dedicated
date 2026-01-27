//
// Created by white on 25. 12. 2..
//

#ifndef FPSPROJECTSERVER_GAMEOBJECTMANAGER_H
#define FPSPROJECTSERVER_GAMEOBJECTMANAGER_H


#include <queue>
#include <vector>

#include "GameObject.h"
#include "GameObjectArgument.h"
class GameSession;

class GameObjectManager {
    protected:
    struct EntitySlot {
        uint32_t generation;
        bool isActive = false;
        GameObjectArgument argument;
    };
    std::vector<GameObjectArgument> objects;

    ///삭제된 GameObject들의 인덱스 위치들
    std::queue<uint32_t>freeIndices;
    public:

    GameSession *ownerSession;

    GameObject CreateGameObject() {
        if (freeIndices.empty()) {
            uint32_t idx = objects.size();
            auto obj = GameObjectArgument(idx, 1);
            obj.gameSession = ownerSession;
            objects.push_back(obj);
            GameObject handle = GameObject(idx,1);
            return handle;
        }
        else {
            uint32_t index = freeIndices.front();
            freeIndices.pop();
            uint32_t gen = objects[index].generationId++;
            GameObject handle = GameObject(index,gen);
            return handle;
        }
    };
    GameObjectArgument* GetGameObject(const GameObject ref) {
        if (ref.GetId() < objects.size()) {
            auto* obj = &objects[ref.GetId()];
            if (obj->generationId != ref.GetGenerationId()) return nullptr;
            return obj;
        }
        else
            return nullptr;
    }
};
#endif //FPSPROJECTSERVER_GAMEOBJECTMANAGER_H