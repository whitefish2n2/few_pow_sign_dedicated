//
// Created by white on 25. 5. 15.
//

#pragma once
#include <map>
#include <memory>
#include <queue>


#include "Map.h"
#include "../Dto/MapEnum.h"
class MapManager
{

    public:
    static MapManager* GetInstance()
    {
        static MapManager instance;
        return &instance;
    }
    void Init();

    Map GetMap(MapEnum type);


    MapManager(const MapManager&) = delete;
    MapManager& operator=(const MapManager&) = delete;
    MapManager(MapManager&&) = delete;
    MapManager& operator=(MapManager&&) = delete;
private:
    std::map<MapEnum, std::unique_ptr<Map>> mapTemplates;

    std::unique_ptr<Map> LoadMap(MapEnum type);

    MapManager() = default;
    ~MapManager() = default;
};