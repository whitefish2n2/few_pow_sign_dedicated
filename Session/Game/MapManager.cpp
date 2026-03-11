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
    obj->transform.SetPosition(pos);
    obj->transform.SetRotation(rot);
}

Layer ParseLayer(const std::string& str, LayerManager& layerManager) {
    return layerManager.toLayer(str);
}


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
            currentObj.components.push_back(constructor);
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

        // 파싱 모드 변경
        if (line == "[SECTION: LAYERS]") {
            currentMode = ParseMode::Layers;
            continue;
        }
        else if (line == "[SECTION: OBJECTS]") {
            currentMode = ParseMode::Objects;
            continue;
        }

        if (currentMode == ParseMode::Layers) {
            // ... (기존 Layer 파싱 코드 동일 유지) ...
            if (line.rfind("LAYER_DEF: ", 0) == 0) {
                std::string data = line.substr(11);
                std::stringstream ss(data);
                std::string segment;
                std::vector<std::string> parts;

                while(std::getline(ss, segment, ',')) {
                    parts.push_back(segment);
                }

                if (parts.size() >= 3) {
                    int idx = std::stoi(parts[0]);
                    const std::string& name = parts[1];
                    auto mask = static_cast<uint32_t>(std::stoll(parts[2]));

                    Layer layer(idx);
                    layerManager.SetLayerInfo(layer, name, mask);
                }
            }
        }
        else if (currentMode == ParseMode::Objects) {
            // '-' 기호를 객체 간의 구분자로 사용
            if (line == "-") {
                FlushComponent();
                if (!currentObj.name.empty() || currentObj.components.size() > 0) {
                    newPhysicsConstructor->InsertObject(std::make_unique<ObjectConstructor>(currentObj));
                }
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
                // GameObject 기본 속성 파싱
                size_t delimPos = line.find(": ");
                if (delimPos != std::string::npos) {
                    std::string key = line.substr(0, delimPos);
                    std::string val = line.substr(delimPos + 2);

                    if (key == "Name") currentObj.name = val;
                    else if (key == "Tag") currentObj.tag = TagManager::GetObjectTagFromString(val);
                    else if (key == "LayerName") currentObj.layer = layerManager.toLayer(val);
                    else if (key == "LayerIndex") currentObj.layer = Layer(std::stoi(val));
                    else if (key == "Position") currentObj.transform.SetPosition(Vector3::ParseVector3(val));
                    else if (key == "Rotation") currentObj.transform.SetRotation(Quaternion::ParseQuaternion(val));
                    else if (key == "Scale") currentObj.transform.SetScale(Vector3::ParseVector3(val));
                }
            } else {
                // 컴포넌트 데이터 누적
                currentCompData << line << "\n";
            }

        }
    }

    FlushComponent();
    if (!currentObj.name.empty() || currentObj.components.size() > 0) {
        newPhysicsConstructor->InsertObject(std::make_unique<ObjectConstructor>(currentObj));
    }

    file.close();

    newPhysicsConstructor->SetLayerManager(std::move(layerManager));

    std::cout << "[MapManager] PhysicsMap.h Loaded: " << path << std::endl;
    std::cout << "[MapManager] Total Objects Parsed: " << newPhysicsConstructor->objects.size() << std::endl; // 잘 파싱되었는지 확인용 (objects 접근 가능할 시)

    return newPhysicsConstructor;
}