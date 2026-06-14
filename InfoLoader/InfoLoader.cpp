#include "InfoLoader.h"
#include <fstream>
#include <iostream>


std::unordered_map<uint32_t, MapInfoArgument> InfoLoader::LoadMapInfo(const std::string& filePath) {
    std::unordered_map<uint32_t, MapInfoArgument> result;
    std::ifstream file;
    file.open(filePath);
    std::cout<<filePath<<std::endl;
    if (!file.is_open()) {
        std::cerr << "[InfoLoader] Error: Cannot open file at " << filePath << std::endl;
        return result;
    }

    try {
        nlohmann::json jsonData;
        file >> jsonData;

        for (const auto& item : jsonData) {
            MapInfoArgument info;
            info.id = item.value("id", 0);
            info.name = item.value("name", "Unknown");
            info.path = item.value("path", "");

            result[info.id] = info;
        }
        std::cout<<"맵파일이 딱잘열려서 딱잘파싱됐소"<< std::endl;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "[InfoLoader] JSON Parse Error in " << filePath << ": " << e.what() << std::endl;
    }

    return result;
}

