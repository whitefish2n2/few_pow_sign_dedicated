//
// Created by white on 26. 2. 3..
//

#ifndef FPSPROJECTSERVER_MAPINFOARGUMENT_H
#define FPSPROJECTSERVER_MAPINFOARGUMENT_H
#include <cstdint>

struct MapInfoArgument {
public:
    uint32_t id = 0;
    std::string name{};
    std::string path{};
};
#endif //FPSPROJECTSERVER_MAPINFOARGUMENT_H