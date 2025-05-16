//
// Created by white on 25. 5. 16.
//
#include "Map.h"
#include <fileapi.h>
#include <fstream>
#include <memory>

std::shared_ptr<Map> Map::LoadMap(MapEnum type)
{
    Map newMap = Map();
    newMap.type = type;
    newMap.triangles = std::vector<int[3]>();
    newMap.vertices = std::vector<Vector3>();
    auto path = GetMapInfoPath(type);

    std::ifstream file(path, std::ios::binary);

    std::string readValue((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

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
    
    return std::make_shared<Map>(newMap);
}
