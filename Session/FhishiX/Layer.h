//
// Created by white on 25. 5. 23.
//

#pragma once
#include <unordered_map>
#include <utility>


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

    std::vector<std::pair<uint32_t, std::string>> GetLayers() {
        std::vector<std::pair<uint32_t, std::string>> layers;
        for (auto v: stringToLayer) {
            layers.push_back(std::make_pair(v.second.idx, v.first));
        }
        return layers;
    }
    Layer toLayer(const std::string& layerName) {
        if (stringToLayer.contains(layerName)) return stringToLayer[layerName];
        return Layer(0);
    }

    uint32_t GetMask(const std::string& layerName) {
        if (!stringToLayer.contains(layerName)) return 0;
        return (1 << stringToLayer[layerName].idx);
    }

    // 사용법: layerManager.GetMask({"Ground", "Default", "Gun"})
    uint32_t GetMask(const std::vector<std::string>& layerNames) {
        uint32_t mask = 0;
        for (const auto& name : layerNames) {
            if (stringToLayer.contains(name)) {
                mask |= (1 << stringToLayer[name].idx);
            }
        }
        return mask;
    }

    void SetLayerInfo(Layer Layer, const std::string& name, uint32_t mask) {
        if (Layer.idx < 0 || Layer.idx>= 32) return;
        layerNames[Layer] = name;
        collisionMasks[Layer.idx] = mask;
        stringToLayer[name] = Layer;
    }
};
struct LayerMask {
    uint32_t value = 0;

    // 기본 생성자
    LayerMask() = default;
    LayerMask(uint32_t val) : value(val) {}

    // 사용법: LayerMask::Get(layer1, layer2, layer3)
    template<typename... Args>
    static LayerMask Get(const Args&... layers) {
        return LayerMask(((1 << layers.idx) | ...));
    }

    void Add(const Layer& layer) {
        value |= (1 << layer.idx);
    }

    void Remove(const Layer& layer) {
        value &= ~(1 << layer.idx);
    }

    bool Contains(const Layer& layer) const {
        return (value & (1 << layer.idx)) != 0;
    }

    LayerMask operator~() const {
        return LayerMask(~value);
    }
    operator uint32_t() const {
        return value;
    }
};

