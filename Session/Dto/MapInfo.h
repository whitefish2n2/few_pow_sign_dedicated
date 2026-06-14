//
// Created by white on 25. 5. 12.
//

#pragma once
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

#include "MapInfoArgument.h"
#include "../../InfoLoader/InfoLoader.h"


struct MapInfo;


class MapRegister {
public:
    //main()에서 호출, MapInfo.json 파일을 불러와서 맵 리스트,를 초기화합니다.
    static void Init(const std::string& MapInfoFilePath) {
        //MapInfo.json 가져와서 idToInfo 초기화
        idToInfo = InfoLoader::LoadMapInfo(MapInfoFilePath);

    }
    static const std::string& GetPath(uint32_t id);
    static const std::string& GetPath(const MapInfo &Info) ;
    static const std::string& GetName(uint32_t id);
    static const std::string& GetName(const MapInfo &Info);
private:
    inline static std::unordered_map<uint32_t, MapInfoArgument> idToInfo;
};

struct MapInfo {
    explicit MapInfo(const uint32_t id):id(id) {};
    MapInfo()=default;
    [[nodiscard]] std::string GetPath() const {
        return MapRegister::GetPath(id);
    }
    [[nodiscard]] std::string GetName() const {
        return MapRegister::GetName(id);
    }
    [[nodiscard]] uint32_t GetID() const {
        return id;
    }
    bool operator==(const MapInfo& other) const {
        return id == other.id;
    }
    bool operator<(const MapInfo& other) const {
        return id < other.id;
    }
private:
    uint32_t id = 0;
};
namespace std {
    template<> struct hash<MapInfo> {
        size_t operator()(const MapInfo& k) const noexcept {
            return hash<int>()(static_cast<int32_t>(k.GetID()));
        }
    };
}
