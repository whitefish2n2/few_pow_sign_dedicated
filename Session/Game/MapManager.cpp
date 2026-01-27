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
void MapManager::Init() {

}

PhysicsSystem MapManager::CreatePhysicsMap(MapInfo type,GameSession* session )
{
    const auto it = mapTemplates.find(type);
    if (it != mapTemplates.end())
    {

        auto loaded = LoadMap(type, session);

        auto inserted = mapTemplates.emplace(type, std::move(loaded));
        return *inserted.first->second;
    }

    return *it->second;
}
void SetupCommonProperties(const GameObject &obj, const std::string& name, const std::string& tagStr, const Layer layer, const Vector3& pos, const Quaternion &rot) {
    obj->name = name;
    obj->tag = TagManager::GetObjectTagFromString(tagStr);
    obj->layer = layer;
    obj->transform.position = pos;
    obj->transform.rotation = rot;
}
// 문자열 "x,y,z"를 Vector3로 변환
Vector3 ParseVector3(const std::string& str) {
    float x, y, z;
    // C# 포맷이 "F4,F4,F4" (콤마 구분)이므로 sscanf로 파싱
    if(sscanf_s(str.c_str(), "%f,%f,%f", &x, &y, &z) == 3) {
        return Vector3(x, y, z);
    }
    return Vector3::Zero();
}

// 문자열 "x,y,z,w"를 Quaternion으로 변환
Quaternion ParseQuaternion(const std::string& str) {
    float x, y, z, w;
    if(sscanf_s(str.c_str(), "%f,%f,%f,%f", &x, &y, &z, &w) == 4) {
        return {x, y, z, w};
    }
    return Quaternion::Identity;
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
                                const std::vector<int>& triangleIndices) {
    GameObject obj = gameObjectManagerInstance->CreateGameObject();
    obj->name = name;
    obj->tag = TagManager::GetObjectTagFromString(tagStr);
    obj->layer = Layer();//todo
    auto v = obj->AddComponent<MeshCollider>(false,vertices, triangleIndices);
    return obj;
};

std::unique_ptr<PhysicsSystem> MapManager::LoadMap(MapInfo type, GameSession* targetSession)
{
    auto newPhysicsMap = std::make_unique<PhysicsSystem>(type);
    auto path = MapRegister::GetPath(&type);

    std::ifstream file("./PhysicsMapInfoFile/" + path);
    if (!file.is_open()) {
        std::cerr << "[Error] Failed to open map file: " << path << std::endl;
        return newPhysicsMap;
    }

    std::string line;
    GameObject currentObj = GameObject::NullPTR(); // 현재 처리 중인 오브젝트

    // 컴포넌트 파싱용 버퍼
    std::string currentCompName;
    std::ostringstream currentCompData;

    // --- [Helper: 컴포넌트 생성 및 부착] ---
    auto FlushComponent = [&]() {
        if (currentObj && !currentCompName.empty()) {
            // Factory에 컴포넌트 이름과 데이터(문자열)를 넘겨서 생성
            // ComponentFactory 내부에서 "Radius: 0.5" 등의 데이터를 다시 파싱해야 함
            ComponentHandleBase complete = ComponentFactory::Instance().Create(currentCompName, currentObj, currentCompData.str(),targetSession->componentManager);

            if (!complete.isNull()) {
                // std::cout << "  -> Attached " << currentCompName << std::endl;
            } else {
                std::cerr << "  [Warning] Unknown or Failed Component: " << currentCompName << std::endl;
            }
        }
        // 버퍼 초기화
        currentCompName = "";
        currentCompData.str("");
        currentCompData.clear();
    };
    //레이어 정보 읽기
    LayerManager layerManager;
    while (std::getline(file, line)) {
        //todo: 맵 레이어 정보 파싱해서 LayerManager에 담기
    }
    // 파일 라인별 읽기
    while (std::getline(file, line)) {
        // 공백 라인 스킵
        if (line.empty()) continue;

        // 윈도우 스타일 줄바꿈 문자(\r) 제거
        if (line.back() == '\r') line.pop_back();

        // 1. 새로운 오브젝트 시작 ("-")
        if (line == "-") {
            FlushComponent(); // 이전 오브젝트의 마지막 컴포넌트 처리

            // 새 오브젝트 생성
            currentObj = gameObjectManagerInstance->CreateGameObject();
            continue;
        }

        // 2. 컴포넌트 섹션 시작 ("COMPONENT: Name")
        if (line.rfind("COMPONENT:", 0) == 0) {
            FlushComponent(); // 이전 컴포넌트 데이터 처리

            // "COMPONENT: " 이후 문자열 추출 (길이 10 + 1)
            if (line.length() > 11) {
                currentCompName = line.substr(11);
            }
            continue;
        }

        // 3. 데이터 파싱
        if (currentCompName.empty()) {
            // [Header Mode] 컴포넌트가 나오기 전이므로 GameObject의 기본 속성 설정
            size_t delimPos = line.find(": ");
            if (delimPos != std::string::npos && currentObj) {
                std::string key = line.substr(0, delimPos);
                std::string val = line.substr(delimPos + 2);

                if (key == "Name") {
                    currentObj->name = val;
                }
                else if (key == "Tag") {
                    currentObj->tag = TagManager::GetObjectTagFromString(val);
                }
                else if (key == "Layer") {
                    currentObj->layer = ParseLayer(val, layerManager);
                }
                else if (key == "Position") {
                    currentObj->transform.position = ParseVector3(val);
                }
                else if (key == "Rotation") {
                    currentObj->transform.rotation = ParseQuaternion(val);
                }
                else if (key == "Scale") {
                    currentObj->transform.scale = ParseVector3(val);
                }
            }
        }
        else {
            // [Component Mode] 현재 컴포넌트 데이터를 버퍼에 누적
            // ComponentFactory가 내부적으로 "Radius: ..." 등을 파싱하도록 넘겨줌
            currentCompData << line << "\n";
        }
    }

    // 파일 끝 도달 시 마지막 컴포넌트 처리
    FlushComponent();
    file.close();

    std::cout << "[MapManager] PhysicsMap.h Loaded: " << path << std::endl;
    return newPhysicsMap;
}