//
// Created by white on 25. 11. 3.
//

#ifndef MESHCOLLIDER_H
#define MESHCOLLIDER_H
#include <memory>
#include <vector>
#include <cpprest/asyncrt_utils.h>
#include "Collider.h"
#include "../../../../util/util.h"
#include "../../../Component/Definition/Component.h"
#include "../../quaternion/Quaternion.h"


class MeshCollider final : public Component<MeshCollider,Collider>{
public:
    mutable std::vector<Vector3> vertices;
    mutable std::vector<uint32_t> trianglesIndices;
        MeshCollider(const bool isStatic, const std::vector<Vector3> &vertices, const std::vector<uint32_t> &indices) : Component(isStatic),vertices(vertices),trianglesIndices(indices) {
        this->triangles.clear();
        this->triangles.reserve(indices.size() / 3);
        for (int i = 0; i<indices.size(); i+=3) {
            if (i + 2 >= indices.size()) break;

            uint32_t v0 = indices[i];
            uint32_t v1 = indices[i + 1];
            uint32_t v2 = indices[i + 2];

            this->triangles.emplace_back(v0, v1, v2);
        }
    }
    MeshCollider() = default;
    const std::vector<Vector3>& GetVertices() const {
        return vertices;
    }
    const std::vector<uint32_t>& GetTriangles() const {
        return trianglesIndices;
    }
    AABB GetAABB() const override;
    std::unique_ptr<Collider> clone() const override {
        return std::make_unique<MeshCollider>(*this);
    }

    Vector3 GetAABBSize() const override {
        return Vector3::Zero();
    }

    bool ContainsPoint(const Vector3& point) const override {
        // 로직 비움
        return false;
    }

    bool IntersectsAABB(const GameObjectArgument& other) const override {
        // 로직 비움
        return false;
    }

    void ExpandAABB(const Vector3& point) const override {
        // 로직 비움
    }

    void MergeAABB(const AABB& other) const override {
        // 로직 비움
    }
    void CalculateAABB() const override {

    };
    void Update() override {
        //Log("메쉬콜라이더업데이트해요");
    };
    void ParseFromString(const std::string &arg) override;

#ifdef _WIN64
    Renderer GetRenderer() override;
    #endif
};
#endif //MESHCOLLIDER_H
