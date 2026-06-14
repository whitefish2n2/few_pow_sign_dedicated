//
// Created by white on 26. 1. 23..
//

#include "MapInfo.h"

const std::string & MapRegister::GetPath(uint32_t id) {
    if (!idToInfo.contains(id)) {
        if (!idToInfo.contains(101)) {
            static const std::string fallbackPath = "Assets/DefaultMap.mapfile";
            return fallbackPath;
        }
        return idToInfo.at(101).path;
    }
    return idToInfo[id].path;///에러 맵
}

const std::string &MapRegister::GetPath(const MapInfo& Info) {
    if (!idToInfo.contains(Info.GetID())) {
        return idToInfo[101].path;
    }
    return idToInfo[Info.GetID()].path;///에러 맵
}

const std::string & MapRegister::GetName(uint32_t id) {
    if (!idToInfo.contains(id)) {
        if (!idToInfo.contains(101)) {
            static const std::string fallbackName = "default map";
            return fallbackName;
        }
        return idToInfo.at(101).name;
    }
    return idToInfo[id].name;
}

const std::string &MapRegister::GetName(const MapInfo& Info)  {
    if (!idToInfo.contains(Info.GetID())) {
        return idToInfo[101].name;
    }
    return idToInfo[Info.GetID()].name;
}
