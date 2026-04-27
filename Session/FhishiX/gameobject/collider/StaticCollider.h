//
// Created by white on 26. 1. 20..
//

#ifndef FPSPROJECTSERVER_STATICCOLLIDER_H
#define FPSPROJECTSERVER_STATICCOLLIDER_H
#include <iostream>

#include "Collider.h"
#include "../../../Component/Definition/Component.h"
#include "../../../Game/KDTree.h"
///사용중지 - PhysicsSystem으로 이관(KDTree의 역할
class StaticCollider {
   /*
   KDTree tree = KDTree();
   public:
   void Build(std::vector<ComponentHandle<Collider>>& colliders) {
      tree.Build(colliders);
   }
   void GetOverlaps(const AABB& queryAABB, std::vector<ComponentHandle<Collider>>& outOverlaps) const;
   AABB GetAABB() const override{ return AABB::Empty();};
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
   Vector3 CalculateLocalInertia(float mass) const override{return Vector3::Zero();};

#ifdef _WIN64
   Renderer GetRenderer() override;
   [[nodiscard]] std::vector<Renderer> GetRenderers() const {
      std::vector<Renderer> renderers;
      renderers.reserve(tree.objectCount);
      return renderers;
   }


#endif
*/
};
#endif //FPSPROJECTSERVER_STATICCOLLIDER_H