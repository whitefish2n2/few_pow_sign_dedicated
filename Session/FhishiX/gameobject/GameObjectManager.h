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
    std::deque<GameObjectArgument> objects;

    ///삭제된 GameObject들의 인덱스 위치들
    std::queue<uint32_t>freeIndices;
    public:

    GameSession *ownerSession;

    GameObject CreateGameObject() {
        if (freeIndices.empty()) {
            uint32_t idx = objects.size();
            auto obj = GameObjectArgument(idx, 1,ownerSession);
            obj.id = idx;
            objects.push_back(obj);
            GameObject handle = GameObject(idx,1, ownerSession);
            return handle;
        }
        else {
            uint32_t index = freeIndices.front();
            freeIndices.pop();
            uint32_t gen = ++objects[index].generationId;
            GameObject handle = GameObject(index,gen, ownerSession);
            return handle;
        }
    };
    GameObjectArgument* GetGameObject(const GameObject ref) {
        if (ref.GetId() >= objects.size()) {
            std::cout << "[GetGameObject FAIL] ID가 배열 크기를 벗어났습니다! "
                      << "요청 ID: " << ref.GetId()
                      << ", 현재 매니저의 Objects Size: " << objects.size() << "\n";
            return nullptr;
        }

        auto* obj = &objects[ref.GetId()];
        if (obj->generationId != ref.GetGenerationId()) {
            std::cout << "[GetGameObject FAIL] 세대(Generation) 불일치! "
                      << "요청 Gen: " << ref.GetGenerationId()
                      << ", 실제 객체 Gen: " << obj->generationId << "\n";
            return nullptr;
        }

        return obj;
    }
};
#endif //FPSPROJECTSERVER_GAMEOBJECTMANAGER_H