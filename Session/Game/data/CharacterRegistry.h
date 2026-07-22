//
// Created by white on 26. 6. 30..
//

#ifndef FPSPROJECTSERVER_CHARACTERGEGISTRY_H
#define FPSPROJECTSERVER_CHARACTERGEGISTRY_H
#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct CharacterInfo {
    uint8_t  id       = 0;
    uint32_t prefabId = 0;
    int      maxHp    = 0;
    float    speed    = 0;
};

class CharacterRegistry {
public:
    static void Init(const std::string& jsonPath);
    static const CharacterInfo* Get(uint8_t id);   // nullptr if not found
private:
    static std::vector<CharacterInfo> _data;        // indexed by id, read-only after Init
};
#endif //FPSPROJECTSERVER_CHARACTERGEGISTRY_H