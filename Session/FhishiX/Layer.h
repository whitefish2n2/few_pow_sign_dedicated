//
// Created by white on 25. 5. 23.
//

#pragma once
enum class Layers {
    Default,
    Ground,
    Gun,
};

class Layer {
    public:
    int idx;

};
class LayerManager {
    static std::vector<uint32_t> collisionMasks;
    static std::unordered_map<int, std::string> layerNames;
    static void Init() {
        collisionMasks.resize(32,0);
    }
    static void SetLayerInfo(int index, const std::string& name, uint32_t mask) {
        if (index < 0 || index >= 32) return;
        layerNames[index] = name;
        collisionMasks[index] = mask;
    }
    static bool CanCollide(int layerA, int layerB) {
        if (layerA < 0 || layerA >= 32 || layerB < 0 || layerB >= 32) return false;

        return (collisionMasks[layerA] & (1 << layerB)) != 0;
    }

    static std::string GetLayerName(int index) {
        if (layerNames.find(index) != layerNames.end()) return layerNames[index];
        return "Unknown";
    }
};
