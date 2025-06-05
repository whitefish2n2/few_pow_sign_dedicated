#include "MapManager.h"

#include <memory>
#include <memory>
#include <sstream>


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
