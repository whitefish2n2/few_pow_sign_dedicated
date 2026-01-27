//
// Created by white on 26. 1. 23..
//

#include "MapInfo.h"

const std::string & MapRegister::GetPath(uint32_t id) {
    if (!idToInfo.contains(id)) {
        static const std::string empty;
        return empty;
    }
    return idToInfo[id].path;
}

const std::string &MapRegister::GetPath(const MapInfo* Info) {
    if (!idToInfo.contains(Info->GetID())) {
        static const std::string empty;
        return empty;
    }
    return idToInfo[Info->GetID()].path;
}

const std::string & MapRegister::GetName(uint32_t id) {
    if (!idToInfo.contains(id)) {
        static const std::string empty;
        return empty;
    }
    return idToInfo[id].name;
}

const std::string &MapRegister::GetName(const MapInfo *Info)  {
    if (!idToInfo.contains(Info->GetID())) {
        static const std::string empty;
        return empty;
    }
    return idToInfo[Info->GetID()].name;
}
