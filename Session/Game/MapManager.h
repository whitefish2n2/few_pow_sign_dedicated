//
// Created by white on 25. 5. 15.
//

#pragma once
#include <memory>
#include <set>


#include "../Dto/MapInfo.h"
#include "Map/MapConstructer/PhysicsSystemConstructor.h"

class MapManager
{
protected:

    public:
    static MapManager* GetInstance()
    {
        static MapManager instance;
        return &instance;
    }
    void Init();

    PhysicsSystemConstructor *GetPhysicsMapConstructor(MapInfo type);


    MapManager(const MapManager&) = delete;
    MapManager& operator=(const MapManager&) = delete;
    MapManager(MapManager&&) = delete;
    MapManager& operator=(MapManager&&) = delete;
private:
    std::shared_mutex mapMutex;
    std::mutex loadingMutex;
    std::condition_variable_any loadingCV;
    std::set<MapInfo> loadingMaps;
    std::unordered_map<MapInfo, std::unique_ptr<PhysicsSystemConstructor>> mapTemplates;
    static std::unique_ptr<PhysicsSystemConstructor> LoadMap(MapInfo type);


    MapManager() = default;
    ~MapManager() = default;
};
