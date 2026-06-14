//
// Created by white on 26. 5. 26..
//

#ifndef FPSPROJECTSERVER_PREFABMANAGER_H
#define FPSPROJECTSERVER_PREFABMANAGER_H


// PrefabManager.h
#pragma once
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <string>

#include "../Session/Game/Map/MapConstructer/ObjectConstructor.h"

class GameObjectArgument;

class PrefabManager {
public:
    static PrefabManager* GetInstance() {
        static PrefabManager instance;
        return &instance;
    }

    // 서버 시작 시 호출: 폴더 내 모든 프리팹을 동적으로 읽어서 메모리에 등록
    static void Init(const std::string& prefabDirectoryPath);

    // 1. 코드에서 문자열로 스폰하고 싶을 때 (예: "Weapon_M4A1-1")
    static GameObject Instantiate(const std::string &prefabName, class GameSession *session);

    // 2. 패킷 등에서 ID(uint32_t)로 스폰하고 싶을 때
    static GameObject Instantiate(uint32_t prefabId, class GameSession *session);

    static const ObjectConstructor *getAsPrefab(const std::string &prefabName);

private:
    PrefabManager() = default;

    inline static std::shared_mutex prefabMutex = std::shared_mutex();

    // [런타임에 동적으로 채워지는 테이블]
    inline static std::unordered_map<uint32_t, std::unique_ptr<ObjectConstructor>> prefabTemplates = std::unordered_map<uint32_t, std::unique_ptr<ObjectConstructor>>();
    inline static std::unordered_map<std::string, uint32_t> nameToIdMap = std::unordered_map<std::string, uint32_t>();

    static void LoadPrefabFromFile(const std::string& filePath);
};
#endif //FPSPROJECTSERVER_PREFABMANAGER_H