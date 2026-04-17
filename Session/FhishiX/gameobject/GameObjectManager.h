//
// Created by white on 25. 12. 2..
//

#ifndef FPSPROJECTSERVER_GAMEOBJECTMANAGER_H
#define FPSPROJECTSERVER_GAMEOBJECTMANAGER_H

#include <queue>
#include <vector>
#include <unordered_map>
#include <string>
#include <limits>
#include <iostream>

#include "GameObject.h"
#include "GameObjectArgument.h"
class GameSession;

class GameObjectManager {
    protected:
    std::vector<size_t> indexArray;
    static constexpr size_t PENDING_MASK = (size_t)1 << (sizeof(size_t) * 8 - 1);
    static constexpr size_t INVALID_INDEX = (std::numeric_limits<size_t>::max)();

    std::vector<GameObjectArgument> objects;
    std::vector<GameObjectArgument> pendingAdds;

    std::queue<uint32_t> freeIndices;
    std::vector<GameObject> pendingDestroys;
    std::unordered_map<std::string, GameObject> nameIndex;

    uint32_t nextId = 0;

    public:
    GameSession *ownerSession;

    GameObject CreateGameObject() {
        uint32_t id;
        uint32_t gen = 1;

        if (freeIndices.empty()) {
            id = nextId++;
        } else {
            id = freeIndices.front();
            freeIndices.pop();
            // 재사용 시 세대 증가
            if (id < objects.size()) {
                gen = objects[id].generationId + 1;
            }
        }

        if (id >= indexArray.size()) {
            indexArray.resize(id + 1, INVALID_INDEX);
        }

        GameObjectArgument obj(id, gen, ownerSession);
        obj.id = id;

        // 대기실에 밀어넣고 인덱스에 마스크 씌우기
        size_t pendingIdx = pendingAdds.size();
        indexArray[id] = pendingIdx | PENDING_MASK;
        pendingAdds.push_back(std::move(obj));
        auto handle = GameObject(id, gen, ownerSession);
        return handle;
    };

    GameObjectArgument* GetGameObject(const GameObject ref) {
        if (ref.GetId() >= indexArray.size()) {
            std::cout << "[GetGameObject FAIL] ID가 배열 크기를 벗어났습니다! "
                      << "요청 ID: " << ref.GetId()
                      << ", 현재 매니저의 IndexArray Size: " << indexArray.size() << "\n";
            return nullptr;
        }

        size_t idx = indexArray[ref.GetId()];
        if (idx == INVALID_INDEX) {
            return nullptr;
        }

        GameObjectArgument* obj = nullptr;

        // PENDING_MASK 확인하여 본진인지 대기실인지 판별
        if (idx & PENDING_MASK) {
            obj = &pendingAdds[idx & ~PENDING_MASK];
        } else {
            obj = &objects[idx];
        }

        if (obj->generationId != ref.GetGenerationId()) {
            std::cout << "[GetGameObject FAIL] 세대(Generation) 불일치! ";
            return nullptr;
        }

        return obj;
    }

    void DeleteGameObject(const GameObject ref) {
        pendingDestroys.push_back(ref);
    }

    ///오브젝트 생성/삭제 지연 처리 플러쉬 함수(매 Update 끝에 호출 요망)
    void Flush() {
        for (const auto& ref : pendingDestroys) {
            GameObjectArgument* obj = GetGameObject(ref);
            if (obj == nullptr) {
                continue;
            }

            obj->Clear();
            obj->generationId++;

            size_t idx = indexArray[ref.GetId()];
            if (idx != INVALID_INDEX) {
                indexArray[ref.GetId()] = INVALID_INDEX;
                freeIndices.push(ref.GetId());
            }
        }
        pendingDestroys.clear();

        for (auto& pending : pendingAdds) {
            uint32_t id = pending.id;

            if (indexArray[id] == INVALID_INDEX) {
                continue;
            }

            if (id >= objects.size()) {
                objects.resize(id + 1);
            }

            objects[id] = std::move(pending);
            indexArray[id] = id;
        }
        pendingAdds.clear();
    }

    GameObject FindByName(const std::string& name) {
        auto it = nameIndex.find(name);
        if (it != nameIndex.end()) {
            return it->second; // 찾으면 핸들 반환
        }
        return GameObject::NullPTR(); // 없으면 빈 핸들 반환
    }

    void SetObjectName(const GameObject &handle, const std::string& name) {
        if (!handle.IsNull()) {
            handle->name = name;
        }
        nameIndex[name] = handle;
    }
};
#endif //FPSPROJECTSERVER_GAMEOBJECTMANAGER_H