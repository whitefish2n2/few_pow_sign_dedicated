#include "MapManager.h"

#include <fstream>
#include <memory>
#include <sstream>

#include "../SessionContext.h"
#include "../FhishiX/Layer.h"
#include "../FhishiX/ObjectTag.h"
#include "../FhishiX/gameobject/collider/BoxCollider.h"
#include "../FhishiX/gameobject/collider/CapsuleCollider.h"
#include "../FhishiX/gameobject/collider/MeshCollider.h"
#include "../FhishiX/quaternion/Quaternion.h"
#include "../FhishiX/gameobject/GameObjectManager.h"
#include "../FhishiX/gameobject/GameObjectArgument.h"
void MapManager::Init() {

}

Map MapManager::GetMap(MapEnum type)
{
    auto it = mapTemplates.find(type);
    if (it == mapTemplates.end())
    {
        auto loaded = LoadMap(type);

        auto inserted = mapTemplates.emplace(type, std::move(loaded));
        return *inserted.first->second;
    }

    return *it->second;
}
void SetupCommonProperties(const GameObject obj, const std::string& name, const std::string& tagStr, const Layers layer, const Vector3 pos, const Quaternion rot) {
    obj->name = name;
    obj->tag = ObjectTag::GetObjectTagFromString(tagStr);
    obj->layer = layer;
    obj->transform.position = pos;
    obj->transform.rotation = rot;
}
GameObject CreateBoxGameObject(const std::string& name, const std::string& tagStr,
                               const Vector3& pos, const Vector3& size, const Quaternion& rot) {
    // 1. 매니저를 통해 객체 생성 (핸들 반환)
    GameObject obj = gameObjectManagerInstance->CreateGameObject();

    // 2. 기본 속성 설정
    SetupCommonProperties(obj, name, tagStr, Layers::Ground, pos, rot);
    obj->type = ObjectTypeEnum::Box;
    obj->transform.scale = size;

    // 3. 컴포넌트 부착
    // AddComponent는 내부적으로 매니저를 호출하고 핸들을 리스트에 넣습니다.
    // BoxCollider 생성자에 맞는 인자를 전달합니다.
    obj->AddComponent<BoxCollider>(false,Vector3::Zero(), Vector3::Zero());
    BoxCollider a = BoxCollider(*obj->GetComponent<BoxCollider>().operator->());
    return obj;
}
GameObject CreateCapsuleGameObject(const std::string& name, const std::string& tagStr,
                                  const Vector3& pos, const Quaternion &rot, const float radius, const float height) {
    GameObject obj = gameObjectManagerInstance->CreateGameObject();
    obj->name = name;
    obj->tag = ObjectTag::GetObjectTagFromString(tagStr);
    obj->type = ObjectTypeEnum::Capsule;
    obj->layer = Layers::Ground;
    ComponentHandle<CapsuleCollider> component = componentManagerInstance->CreateComponentAtPool<CapsuleCollider>(true,pos,height,radius);
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
    obj->tag = ObjectTag::GetObjectTagFromString(tagStr);
    obj->type = ObjectTypeEnum::Mesh;
    obj->layer = Layers::Ground;
    auto v = obj->AddComponent<MeshCollider>(false,vertices, triangleIndices);
    return obj;
};

GameObject ParseGameObjectFromRawFormat(const std::string& raw) {
    std::istringstream ss(raw);
    std::string line;

    ///대충 컴포넌트 맞춰서 생성했석
    //ID
    std::getline(ss, line);
    // 태그
    std::getline(ss, line);
    obj->tag = ObjectTag::GetObjectTagFromString(line);

    // 빈 줄 스킵
    while (std::getline(ss, line) && line.empty())

    // vertices
    do {
        if (line.empty()) break;
        std::stringstream ls(line);
        float x, y, z;
        char comma;
        ls >> x >> comma >> y >> comma >> z;
        obj.vertices.emplace_back(x, y, z);
    } while (std::getline(ss, line) && !line.empty());

    // triangles
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        std::stringstream ls(line);
        int a, b, c;
        char comma;
        ls >> a >> comma >> b >> comma >> c;
        obj.triangles.push_back(Triangle{a, b, c});
    }

    return obj;
}

std::unique_ptr<Map> MapManager::LoadMap(MapEnum type)
{
    auto newMap = Map(type);
    auto path = GetMapInfoPath(type);

    std::ifstream file("./MapInfoFile/"+path, std::ios::binary);

    std::string readValue((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();


    std::vector<GameObject> result;
    std::stringstream ss(readValue);
    std::string block;
    std::string line;
    std::ostringstream currentBlock;

    while (std::getline(ss, line)) {
        if (line == "-") {
            auto obj = ParseGameObjectFromRawFormat(currentBlock.str());
            newMap.objects[obj->id] = obj;
            currentBlock.str(""); // 리셋
            currentBlock.clear();
        } else {
            currentBlock << line << "\n";
        }
    }
    if (!currentBlock.str().empty()) {
        result.push_back(ParseGameObjectFromRawFormat(currentBlock.str()));
    }

    /*legacy
     *std::string readValue;
    FILE *file = nullptr;
    if (0==fopen_s(&file, path.c_str(), "rb"))
    {

        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        rewind(file);

        std::vector<char> buffer(size + 1); // +1은 널 문자 용
        fread(buffer.data(), 1, size, file);
        buffer[size] = '\0'; // 문자열로 만들기 위해 종료 문자 추가

        readValue = std::string(buffer.data());
        fclose(file);
    }*/
    return std::make_unique<Map>(newMap);
}
