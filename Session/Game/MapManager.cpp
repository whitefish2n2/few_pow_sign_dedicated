#include "MapManager.h"

#include <fstream>
#include <memory>
#include <sstream>

#include "../SessionContext.h"
#include "../Component/Definition/ComponentFactory.h"
#include "../FhishiX/Layer.h"
#include "../FhishiX/TagManager.h"
#include "../FhishiX/gameobject/collider/BoxCollider.h"
#include "../FhishiX/gameobject/collider/CapsuleCollider.h"
#include "../FhishiX/gameobject/collider/MeshCollider.h"
#include "../FhishiX/quaternion/Quaternion.h"
#include "../FhishiX/gameobject/GameObjectManager.h"
#include "../FhishiX/gameobject/GameObjectArgument.h"
#include "Map/MapConstructer/PhysicsSystemConstructor.h"

void MapManager::Init() {

}

PhysicsSystemConstructor *MapManager::GetPhysicsMapConstructor(MapInfo type)
{
    auto it = mapTemplates.find(type);
    if (it == mapTemplates.end())
    {
        auto loaded = LoadMap(type);
        auto res = mapTemplates.emplace(type, std::move(loaded));
        it = res.first;
    }

    return it->second.get();
}
void SetupCommonProperties(const GameObject &obj, const std::string& name, const std::string& tagStr, const Layer layer, const Vector3& pos, const Quaternion &rot) {
    obj->name = name;
    obj->tag = TagManager::GetObjectTagFromString(tagStr);
    obj->layer = layer;
    obj->transform.position = pos;
    obj->transform.rotation = rot;
}

Layer ParseLayer(const std::string& str, LayerManager& layerManager) {
    return layerManager.toLayer(str);
}

GameObject CreateBoxGameObject(const std::string& name, const std::string& tagStr,
                               const Vector3& pos, const Vector3& size, const Quaternion& rot) {
    // 1. 매니저를 통해 객체 생성 (핸들 반환)
    GameObject obj = gameObjectManagerInstance->CreateGameObject();

    // 2. 기본 속성 설정
    SetupCommonProperties(obj, name, tagStr, Layer(), pos, rot);
    obj->transform.scale = size;

    // 3. 컴포넌트 부착
    // AddComponent는 내부적으로 매니저를 호출하고 핸들을 리스트에 넣습니다.
    // BoxCollider 생성자에 맞는 인자를 전달합니다.
    obj->AddComponent<BoxCollider>(false,Vector3::Zero(), Vector3::Zero());
    return obj;
}
GameObject CreateCapsuleGameObject(const std::string& name, const std::string& tagStr,
                                  const Vector3& pos, const Quaternion &rot, const float radius, const float height) {
    GameObject obj = gameObjectManagerInstance->CreateGameObject();
    obj->name = name;
    obj->tag = TagManager::GetObjectTagFromString(tagStr);
    obj->layer = Layer();//todo
    ComponentHandle<CapsuleCollider> component = componentManagerInstance->CreateComponentAtPool<CapsuleCollider>(true,pos,height,radius);
    componentManagerInstance->CreateComponentAtPool<CapsuleCollider>();
    component->haveMesh = false;
    obj->transform = Transform();
    obj->transform.position = pos;
    obj->transform.rotation = rot;
    return obj;
}
GameObject CreateMeshGameObject(const std::string& name, const std::string& tagStr,
                                const std::vector<Vector3>& vertices,
                                const std::vector<uint32_t>& triangleIndices) {
    GameObject obj = gameObjectManagerInstance->CreateGameObject();
    obj->name = name;
    obj->tag = TagManager::GetObjectTagFromString(tagStr);
    obj->layer = Layer();//todo
    auto v = obj->AddComponent<MeshCollider>(false,vertices, triangleIndices);
    return obj;
};

std::unique_ptr<PhysicsSystemConstructor> MapManager::LoadMap(MapInfo type)
{
    auto newPhysicsConstructor = std::make_unique<PhysicsSystemConstructor>();
    auto path = MapRegister::GetPath(&type);

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[Error] Failed to open map file: " << path << std::endl;
        return newPhysicsConstructor;
    }

    std::string line;
    ParseMode currentMode = ParseMode::None;

    //레이어 파싱용 객체
    LayerManager layerManager = LayerManager();
    layerManager.Init();

    ObjectConstructor currentObj = ObjectConstructor(); // 현재 처리 중인 오브젝트

    // 컴포넌트 파싱용 버퍼
    std::string currentCompName;
    std::ostringstream currentCompData;

    // --- [Helper: 컴포넌트 생성 및 부착] ---
    auto FlushComponent = [&]() {
        if (!currentCompName.empty()) {
            ComponentConstructor constructor = ComponentConstructor(currentCompName,currentCompData.str());
        }
        // 버퍼 초기화
        currentCompName = "";
        currentCompData.str("");
        currentCompData.clear();
    };
    // 파일 라인별 읽기
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        //파싱 모드 변경
        if (line == "[SECTION: LAYERS]") {
            currentMode = ParseMode::Layers;
            continue;
        }
        else if (line == "[SECTION: OBJECTS]") {
            currentMode = ParseMode::Objects;
            continue;
        }

        if (currentMode == ParseMode::Layers) {
            // 포맷: LAYER_DEF: index,name,mask
            if (line.rfind("LAYER_DEF: ", 0) == 0) {
                std::string data = line.substr(11); // "LAYER_DEF: " 길이
                std::stringstream ss(data);
                std::string segment;
                std::vector<std::string> parts;

                while(std::getline(ss, segment, ',')) {
                    parts.push_back(segment);
                }

                if (parts.size() >= 3) {
                    int idx = std::stoi(parts[0]);
                    const std::string& name = parts[1];
                    auto mask = static_cast<uint32_t>(std::stoll(parts[2])); // int 범위를 넘을 수 있으므로 stoll 후 캐스팅

                    Layer layer(idx);
                    layerManager.SetLayerInfo(layer, name, mask);
                    // std::cout << "Loaded Layer: " << idx << " (" << name << ")" << std::endl;
                }
            }
        }
        else if (currentMode == ParseMode::Objects) {
            // 오브젝트 파싱
            if (line == "-") {
                FlushComponent();
                currentObj = ObjectConstructor();
                continue;
            }

            if (line.rfind("COMPONENT:", 0) == 0) {
                FlushComponent();
                if (line.length() > 11) {
                    currentCompName = line.substr(11);
                }
                continue;
            }

            if (currentCompName.empty()) {
                // GameObject 속성 파싱
                size_t delimPos = line.find(": ");
                if (delimPos != std::string::npos && currentObj.name.empty()) {
                    std::string key = line.substr(0, delimPos);
                    std::string val = line.substr(delimPos + 2);

                    if (key == "Name") currentObj.name = val;
                    else if (key == "Tag") currentObj.tag = TagManager::GetObjectTagFromString(val);
                    else if (key == "LayerName") {
                        currentObj.layer = layerManager.toLayer(val);
                    }
                    else if (key == "LayerIndex") {
                        // 만약 인덱스로 저장했다면 바로 캐스팅
                        currentObj.layer = Layer(std::stoi(val));
                    }
                    else if (key == "Position") currentObj.transform.position = Vector3::ParseVector3(val);
                    else if (key == "Rotation") currentObj.transform.rotation = Quaternion::ParseQuaternion(val);
                    else if (key == "Scale") currentObj.transform.scale = Vector3::ParseVector3(val);
                }
            } else {
                currentCompData << line << "\n";
            }
        }
    }

    // 파일 끝 도달 시 마지막 컴포넌트 처리
    FlushComponent();
    file.close();

    newPhysicsConstructor->SetLayerManager(std::move(layerManager));

    std::cout << "[MapManager] PhysicsMap.h Loaded: " << path << std::endl;
    return newPhysicsConstructor;
}