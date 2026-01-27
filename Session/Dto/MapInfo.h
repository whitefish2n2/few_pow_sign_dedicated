//
// Created by white on 25. 5. 12.
//

#pragma once
#include <string>
#include <unordered_map>


struct MapInfo;

struct MapInfoArgument {
public:
    uint32_t id = 0;
    std::string name;
    std::string path;
};
class MapRegister {
public:
    //main()에서 호출, MapInfo.json 파일을 불러와서 맵 리스트,를 초기화합니다.
    void Init() {
        //MapInfo.json 가져와서 idToInfo 초기화
    }
    static const std::string& GetPath(uint32_t id);
    static const std::string& GetPath(const MapInfo* Info) ;
    static const std::string& GetName(uint32_t id);
    static const std::string& GetName(const MapInfo* Info);
private:
    inline static std::unordered_map<uint32_t, MapInfoArgument> idToInfo;
};

struct MapInfo {
public:
    explicit MapInfo(uint32_t id):id(id) {};
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
    uint32_t id = 0;;
};
namespace std {
    template<> struct hash<MapInfo> {
        size_t operator()(const MapInfo& k) const noexcept {
            return hash<int>()(static_cast<int32_t>(k.GetID()));
        }
    };
}
