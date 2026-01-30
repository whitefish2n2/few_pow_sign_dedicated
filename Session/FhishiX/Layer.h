//
// Created by white on 25. 5. 23.
//

#pragma once
#include <unordered_map>
enum class Layers {
    Default,
    Ground,
    Gun,
};

class Layer {
    public:
    int idx;
    bool operator==(const Layer & other) const {
        return other.idx == this->idx;
    }
    bool operator<(const Layer & other) const {
        return this->idx < other.idx;
    }
};

namespace std {
    template<>
    struct hash<Layer> {
        size_t operator()(const Layer& k) const noexcept {
            return hash<int>()(k.idx);
        }
    };
}

enum class ParseMode {
    None,
    Layers,
    Objects
};
//fix: 기존 enum 레이어 구조는 맵을 새로 만들고 업데이트할때 유리하지 않았고, 따라서 레이어 이름 map 매핑 방식으로 변경해 유연성과 이래저래 높임.
class LayerManager {
    private:
    std::vector<uint32_t> collisionMasks;
    std::unordered_map<Layer, std::string> layerNames;
    std::unordered_map<std::string, Layer> stringToLayer;
    public:
    void Init() {
        collisionMasks.resize(32,0);
    }
    bool CanCollide(int layerA, int layerB) {
        if (layerA < 0 || layerA >= 32 || layerB < 0 || layerB >= 32) return false;

        return (collisionMasks[layerA] & (1 << layerB)) != 0;
    }

    std::string GetLayerName(const Layer index) {
        if (layerNames.contains(index)) return layerNames[index];
        return "Unknown";
    }
    Layer toLayer(const std::string& layerName) {
        if (stringToLayer.contains(layerName)) return stringToLayer[layerName];
        return Layer(0);
    }
    void SetLayerInfo(Layer Layer, const std::string& name, uint32_t mask) {
        if (Layer.idx < 0 || Layer.idx>= 32) return;
        layerNames[Layer] = name;
        collisionMasks[Layer.idx] = mask;
        stringToLayer[name] = Layer;
    }



};
