//
// Created by white on 25. 5. 15.
//

#pragma once
#include <map>
#include <memory>


#include "PhysicsSystem.h"
#include "../Dto/MapInfo.h"
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

    PhysicsSystem CreatePhysicsMap(MapInfo type, GameSession *session);


    MapManager(const MapManager&) = delete;
    MapManager& operator=(const MapManager&) = delete;
    MapManager(MapManager&&) = delete;
    MapManager& operator=(MapManager&&) = delete;
private:
    std::unordered_map<MapInfo, std::unique_ptr<PhysicsSystem>> mapTemplates;
    static std::unique_ptr<PhysicsSystem> LoadMap(MapInfo type, GameSession *targetSession);


    MapManager() = default;
    ~MapManager() = default;
};
