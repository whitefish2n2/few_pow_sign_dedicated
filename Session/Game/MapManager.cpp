#include "MapManager.h"

#include <fstream>
#include <memory>
#include <sstream>
#include "../FhishiX/gameobject/Collider/BoxCollider.h"
#include "../FhishiX/gameobject/Collider/CapsuleCollider.h"

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
GameObject CreateBoxGameObject(const std::string& name, const std::string& tagStr,
                               const Vector3& pos, const Vector3& size) {
    GameObject obj = GameObject();
    obj.id = name;
    obj.tag = ObjectTag::GetObjectTagFromString(tagStr);
    obj.type = ObjectTypeEnum::Box;
    obj.layer = Layers::Ground;

    // 박스의 8개 버텍스 생성
    float hx = size.x * 0.5f;
    float hy = size.y * 0.5f;
    float hz = size.z * 0.5f;

    obj.vertices = {
        Vector3(pos.x - hx, pos.y - hy, pos.z - hz), // 0
        Vector3(pos.x + hx, pos.y - hy, pos.z - hz), // 1
        Vector3(pos.x + hx, pos.y + hy, pos.z - hz), // 2
        Vector3(pos.x - hx, pos.y + hy, pos.z - hz), // 3
        Vector3(pos.x - hx, pos.y - hy, pos.z + hz), // 4
        Vector3(pos.x + hx, pos.y - hy, pos.z + hz), // 5
        Vector3(pos.x + hx, pos.y + hy, pos.z + hz), // 6
        Vector3(pos.x - hx, pos.y + hy, pos.z + hz)  // 7
    };

    // 박스의 12개 삼각형 (6면 x 2삼각형)
    obj.triangles = {
        Triangle{0, 2, 1}, Triangle{0, 3, 2}, // Front
        Triangle{1, 2, 6}, Triangle{1, 6, 5}, // Right
        Triangle{4, 5, 6}, Triangle{4, 6, 7}, // Back
        Triangle{0, 7, 3}, Triangle{0, 4, 7}, // Left
        Triangle{3, 7, 6}, Triangle{3, 6, 2}, // Top
        Triangle{0, 1, 5}, Triangle{0, 5, 4}  // Bottom
    };

    obj.CalculateAABB();
    return obj;
}
GameObject CreateCapsuleGameObject(const std::string& name, const std::string& tagStr,
                                  const Vector3& pos, const float radius, const float height) {
    GameObject obj = GameObject();
    obj.id = name;
    obj.tag = ObjectTag::GetObjectTagFromString(tagStr);
    obj.type = ObjectTypeEnum::Capsule;
    obj.layer = Layers::Ground;
    obj.collider = std::make_unique<CapsuleCollider>(CapsuleCollider(&obj, true, {0,0,0}, height, radius));

    obj.transform.position = pos;
    obj.transform.rotation =

    return obj;
}
GameObject CreateMeshGameObject(const std::string& name, const std::string& tagStr,
                                const std::vector<Vector3>& vertices,
                                const std::vector<int>& triangleIndices) {
    GameObject obj;
    obj.id = name;
    obj.tag = ObjectTag::GetObjectTagFromString(tagStr);
    obj.type = ObjectTypeEnum::Mesh;
    obj.layer = Layers::Ground;

    obj.vertices = vertices;

    // 인덱스를 Triangle 구조체로 변환
    for (size_t i = 0; i < triangleIndices.size(); i += 3) {
        obj.triangles.push_back(Triangle{
            triangleIndices[i],
            triangleIndices[i + 1],
            triangleIndices[i + 2]
        });
    }

    obj.CalculateAABB();
    return obj;
}
GameObject ParseGameObjectFromRawFormat(const std::string& raw) {
    std::istringstream ss(raw);
    std::string line;

    GameObject obj;

    //ID
    std::getline(ss, line);
    obj.id = line;
    // 태그
    std::getline(ss, line);
    obj.tag = ObjectTag::GetObjectTagFromString(line);

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
            newMap.objects[obj.id] = obj;
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
