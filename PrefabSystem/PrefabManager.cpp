//
// Created by white on 26. 5. 26..
//

#include "PrefabManager.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>

#include "../util/StringUtil.h"
#include "../Session/FhishiX/TagManager.h"
#include "../Session/FhishiX/Layer.h"
void PrefabManager::Init(const std::string& prefabDirectoryPath) {

    namespace fs = std::filesystem;
    if (!fs::exists(prefabDirectoryPath)) {
        std::cerr << "[PrefabManager][Error] Directory does not exist: " << prefabDirectoryPath << std::endl;
        return;
    }

    int successCount = 0;
    for (const auto& entry : fs::directory_iterator(prefabDirectoryPath)) {
        if (entry.path().extension() == ".objectPrefab") {
            LoadPrefabFromFile(entry.path().string());
            successCount++;
        }
    }

    std::cout << "[PrefabManager] Initialization complete. Total " << successCount << " prefabs dynamicly registered." << std::endl;
}

void PrefabManager::LoadPrefabFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[PrefabManager][Error] Failed to open prefab file: " << filePath << std::endl;
        return;
    }

    std::string line;
    uint32_t parsedPrefabId = 0;
    std::string prefabName = "";

    enum class ParseMode { None, Header, Objects };
    ParseMode currentMode = ParseMode::None;

    auto rootConstructor = std::make_unique<ObjectConstructor>();
    ObjectConstructor* currentObj = nullptr;

    std::string currentCompName;
    std::ostringstream currentCompData;

    // 컴포넌트 버퍼 플러시 헬퍼 람다
    auto FlushComponent = [&]() {
        if (!currentCompName.empty() && currentObj) {
            ComponentConstructor constructor(currentCompName, currentCompData.str());
            currentObj->components.push_back(constructor);
        }
        currentCompName = "";
        currentCompData.str("");
        currentCompData.clear();
    };

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        // 섹션 전환 확인
        if (line == "[SECTION: HEADER]") {
            currentMode = ParseMode::Header;
            continue;
        }
        else if (line == "[SECTION: OBJECTS]") {
            currentMode = ParseMode::Objects;
            continue;
        }

        // 1. 헤더 파싱 단계
        if (currentMode == ParseMode::Header) {
            size_t delimPos = line.find(':');
            if (delimPos == std::string::npos) continue;

            std::string key = StringUtils::Trim(line.substr(0, delimPos));
            std::string val = StringUtils::Trim(line.substr(delimPos + 1));

            if (key == "PrefabID") {
                parsedPrefabId = std::stoul(val);
            }
            else if (key == "PrefabName") {
                prefabName = val;
            }
        }
        // 2. 오브젝트 및 컴포넌트 파싱 단계
        else if (currentMode == ParseMode::Objects) {
            if (line == "-") {
                FlushComponent();
                // 첫 오브젝트(-) 등장 시 루트로 지정, 자식이 있다면 평탄화 형태로 추가 확장 가능
                currentObj = rootConstructor.get();
                continue;
            }

            if (line.rfind("COMPONENT:", 0) == 0) {
                FlushComponent();
                currentCompName = StringUtils::Trim(line.substr(10));
                continue;
            }

            if (currentObj) {
                if (currentCompName.empty()) {
                    // 기본 Transform 및 GameObject 속성 파싱
                    size_t delimPos = line.find(':');
                    if (delimPos == std::string::npos) continue;

                    std::string key = StringUtils::Trim(line.substr(0, delimPos));
                    std::string val = StringUtils::Trim(line.substr(delimPos + 1));

                    if (key == "Name") currentObj->name = val;
                    else if (key == "Tag") currentObj->tag = TagManager::GetObjectTagFromString(val);
                    else if (key == "LayerIndex") currentObj->layer = Layer(std::stoi(val));
                    else if (key == "Position") currentObj->transform.SetPosition(Vector3::ParseVector3(val));
                    else if (key == "Rotation") currentObj->transform.SetRotation(Quaternion::ParseQuaternion(val));
                    else if (key == "Scale") currentObj->transform.SetScale(Vector3::ParseVector3(val));
                }
                else {
                    // 컴포넌트 데이터 문자열 누적
                    currentCompData << line << "\n";
                }
            }
        }
    }

    FlushComponent();
    file.close();

    // 데이터 유효성 검증 후 런타임 맵에 동적 등록 (Write Lock 소유)
    if (parsedPrefabId > 0 && !prefabName.empty()) {
        std::unique_lock<std::shared_mutex> lock(prefabMutex);

        nameToIdMap[prefabName] = parsedPrefabId;
        prefabTemplates[parsedPrefabId] = std::move(rootConstructor);
    } else {
        std::cerr << "[PrefabManager][Warning] Invalid prefab data format in: " << filePath << std::endl;
    }
}

GameObject PrefabManager::Instantiate(const std::string &prefabName, GameSession *session) {
    uint32_t prefabId = 0;

    // 이름으로 ID 조회 (Read Lock 소유)
    {
        std::shared_lock<std::shared_mutex> lock(prefabMutex);
        auto it = nameToIdMap.find(prefabName);
        if (it != nameToIdMap.end()) {
            prefabId = it->second;
        }
    }

    if (prefabId == 0) {
        std::cerr << "[PrefabManager][Error] Failed to find prefab name: " << prefabName << std::endl;
        return GameObject::NullPTR(); // 유효하지 않은 경우 안전하게 빈 객체 반환
    }

    return Instantiate(prefabId, session);
}

GameObject PrefabManager::Instantiate(uint32_t prefabId, GameSession *session) {
    const ObjectConstructor* templatePtr = nullptr;

    // ID로 템플릿 탐색 (Read Lock 소유)
    {
        std::shared_lock<std::shared_mutex> lock(prefabMutex);
        auto it = prefabTemplates.find(prefabId);
        if (it != prefabTemplates.end()) {
            templatePtr = it->second.get();
        }
    }

    if (!templatePtr) {
        std::cerr << "[PrefabManager][Error] Failed to find prefab template ID: " << prefabId << std::endl;
        return GameObject::NullPTR();
    }

    auto newObj = templatePtr->Construct(session);

    return newObj;
}

const ObjectConstructor *PrefabManager::getAsPrefab(const std::string &prefabName) {
    uint32_t prefabId = 0;

    // 이름으로 ID 조회 (Read Lock 소유)
    {
        std::shared_lock<std::shared_mutex> lock(prefabMutex);
        auto it = nameToIdMap.find(prefabName);
        if (it != nameToIdMap.end()) {
            prefabId = it->second;
        }
    }

    if (prefabId == 0) {
        std::cerr << "[PrefabManager][Error] Failed to find prefab name: " << prefabName << std::endl;
        return nullptr;;//유효하지 않은 경우 안전하게 빈 객체 반환
    }
    const ObjectConstructor* templatePtr = nullptr;

    // ID로 템플릿 탐색 (Read Lock 소유)
    {
        std::shared_lock<std::shared_mutex> lock(prefabMutex);
        auto it = prefabTemplates.find(prefabId);
        if (it != prefabTemplates.end()) {
            templatePtr = it->second.get();
        }
    }

    if (!templatePtr) {
        std::cerr << "[PrefabManager][Error] Failed to find prefab template ID: " << prefabId << std::endl;
        return nullptr;
    }
    return templatePtr;
}
