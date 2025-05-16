#include "Map.h"
#include "../Dto/MapEnum.h"
#include "MapManager.h"
//
// Created by white on 25. 5. 15.
//
std::shared_ptr<Map> MapManager::GetMap(MapEnum type)
{
    if (maps[type].size() > 0)
    {
        auto map = maps[type].front();
        map->Init();
        return map;
    }
    else
    {
        auto map = Map();
        map.LoadMap(type);

    }

}
