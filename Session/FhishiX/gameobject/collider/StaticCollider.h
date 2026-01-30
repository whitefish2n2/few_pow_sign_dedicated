//
// Created by white on 26. 1. 20..
//

#ifndef FPSPROJECTSERVER_STATICCOLLIDER_H
#define FPSPROJECTSERVER_STATICCOLLIDER_H
#include "Collider.h"
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
};
#endif //FPSPROJECTSERVER_STATICCOLLIDER_H