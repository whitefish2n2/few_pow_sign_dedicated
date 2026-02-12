//
// Created by white on 26. 2. 3..
//

#ifndef FPSPROJECTSERVER_INFOLOADER_H
#define FPSPROJECTSERVER_INFOLOADER_H
#include <nlohmann/json.hpp>
#include "../Session/Dto/MapInfoArgument.h"
class InfoLoader {
    public:
        static std::unordered_map<uint32_t, MapInfoArgument> LoadMapInfo(const std::string &filePath);
};


#endif //FPSPROJECTSERVER_INFOLOADER_H