//
// Created by white on 25. 5. 15.
//

#pragma once
#include <map>
#include <memory>
#include <queue>

#include "../Dto/MapEnum.h"
#include "Map.h"
class MapManager
{

    public:
    static MapManager* GetInstance()
    {
        static MapManager instance;
        return &instance;
    }

    std::shared_ptr<Map> GetMap(MapEnum type);


    MapManager(const MapManager&) = delete;
    MapManager& operator=(const MapManager&) = delete;
    MapManager(MapManager&&) = delete;
    MapManager& operator=(MapManager&&) = delete;
private:
    std::map<MapEnum, std::queue<std::shared_ptr<Map>>> maps;
    MapManager() = default;
    ~MapManager() = default;
};