//
// Created by white on 26. 1. 20..
//

#ifndef FPSPROJECTSERVER_STATICCOLLIDER_H
#define FPSPROJECTSERVER_STATICCOLLIDER_H
#include <iostream>

#include "Collider.h"
#include "../../../Component/Definition/Component.h"
#include "../../../Game/KDTree.h"

class StaticCollider final:public Component<StaticCollider,Collider> {
   KDTree tree = KDTree();
   const std::vector<Vector3>& GetVertices() const {
      return vertices;
   }
   AABB GetAABB() const override;
   std::unique_ptr<Collider> clone() const override {
      return std::make_unique<StaticCollider>(*this);
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
   void ParseFromString(const std::string &arg) override{ std::cout << "StaticCollider::ParseFromString is not implemented. 어디서 쓰고있으면 큰 인간버그임" << std::endl; };

   ///OnAttach 삭제
   void OnAttach() override {}


#ifdef _WIN64
   Renderer GetRenderer() override;
   [[nodiscard]] std::vector<Renderer> GetRenderers() const {
      std::vector<Renderer> renderers;
      renderers.reserve(tree.objectCount);
      for (auto v: tree) {
         renderers.push_back(v->GetRenderer());
      }
      return renderers;
   }
#endif
};
#endif //FPSPROJECTSERVER_STATICCOLLIDER_H