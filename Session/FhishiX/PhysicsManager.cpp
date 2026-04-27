//
// Created by white on 26. 3. 16..
//
#include "PhysicsManager.h"

#include <future>
#include <vector>
#include <vector>

#include "AABB.h"
#include "../GameSession.h"
#include "../Component/Definition/ComponentFactory.h"
#include "../Component/Definition/ComponentManager.h"
#include "gameobject/collider/Collider.h"
#include "gameobject/rigidBody/Rigidbody.h"
#include "../../Session/Game/PhysicsSystem.h"
#include "gameobject/collider/BoxCollider.h"
REGISTER_COMPONENT(PhysicsManager);
struct CollisionPair {
    Collider* dynamicCollider;
    Rigidbody* dynamicRb;
    // Static 객체는 포인터거나 ID일 수 있음. 여기서는 임시로 void* 처리
    void* staticCollider;
};
struct DynamicCollisionPair {
    Collider* c1;
    Collider* c2;
    Rigidbody* rb1;
    Rigidbody* rb2;
};
struct DynamicProxy {
    AABB bounds;
    Collider* original;
    Rigidbody* rb;
    ComponentHandle<Collider> collider;
    DynamicProxy(Rigidbody* r, Collider* c) : rb(r), original(c) {
        bounds = c->GetAABB();
    }
};
void PhysicsManager::PhysicsUpdate() {
    LOG_DEBUG("PhysicsUpdate 실행");
    DrivenPool<Rigidbody>* rigidBodies = gameSession->componentManager.get()->GetOrCreatePool<Rigidbody>();
    std::vector<DynamicProxy> dynamicProxies;
    dynamicProxies.reserve(rigidBodies->size());

    for (auto& v : *rigidBodies) {
        if (v.isActive && v.gameObject && !v.isKinematic) {
            if (v.entityId >= activeActors.size()) {
                activeActors.resize(v.entityId + 1, PhysicsActor::NULLPTR());
            }
            if (!activeActors[v.entityId].isActive()) {
                PhysicsActor actor;
                actor.rb = v.MakeHandle();
                actor.col = v.gameObject->GetComponent<Collider>(); /// fix:: 매 업데이트마다 추상 객체 GetComponent를 하던 문제를 콜라이더 캐싱으로 해결함.

                activeActors[v.entityId] = actor;
            }
            auto& actor = activeActors[v.entityId];
            /*
            std::string msg1 = "[PhysicsManager] ID: " + std::to_string(v.entityId) +
                   " 검사 -> isActive: " + std::to_string(v.isActive) +
                   " | isKinematic: " + std::to_string(v.isKinematic) +
                   " | hasCollider: " + std::to_string(!actor.col.isNull());
            LOG_DEBUG(msg1);
            */
            if (actor.col.isNull()) continue;
            v.Integrate();

            dynamicProxies.emplace_back(&v, actor.col.operator->());
        }
    }if (!dynamicProxies.empty()) {
        std::vector<CollisionPair> collisionPairs;
        std::vector<DynamicCollisionPair> dynamicCollisionPairs;
        // 보통 하나의 다이나믹 객체가 2~4개의 스태틱 객체와 겹친다고 가정하여 미리 메모리 할당
        collisionPairs.reserve(dynamicProxies.size() * 2);

        for (const auto& proxy : dynamicProxies) {
            const AABB& bounds = proxy.bounds;
            Collider* myCollider = proxy.original;

            thread_local std::vector<ComponentHandle<Collider>> overlapResults;

            overlapResults.clear(); // 사이즈만 0으로 만들고 메모리는 유지
            std::string dbgMsg = "[X-RAY] 트리 객체 수: " + std::to_string(gameSession->physicsSystem->tree.objectCount) +
                                 " | 내 Y위치: " + std::to_string(myCollider->gameObject->transform.GetPosition().y) +
                                 " | AABB Min.y: " + std::to_string(bounds.min.y);
            LOG_DEBUG(dbgMsg);
            // K-D 트리 탐색!
            gameSession->physicsSystem->tree.GetOverlaps(proxy.bounds, overlapResults);
            if (!overlapResults.empty()) {
                std::string msg1 = "[BroadPhase] K-D 트리에서 AABB 겹침 감지! 후보 개수: " + std::to_string(overlapResults.size());
                LOG_DEBUG(msg1);
            }
            for (auto v: overlapResults) {
                CollisionPair collisionPair;
                collisionPair.dynamicCollider = myCollider;
                collisionPair.dynamicRb = proxy.rb;
                collisionPair.staticCollider = v.operator->();
                collisionPairs.push_back(collisionPair);
            }
        }
        for (int i = 0; i<dynamicProxies.size(); i++) {
            for (int j = i+1; j<dynamicProxies.size(); j++) {
                DynamicCollisionPair pair;
                if (dynamicProxies[i].bounds.Intersects(dynamicProxies[j].bounds)) {
                    pair.c1 = dynamicProxies[i].original;
                    pair.c2 = dynamicProxies[j].original;
                    pair.rb1 = dynamicProxies[i].rb;
                    pair.rb2 = dynamicProxies[j].rb;
                    dynamicCollisionPairs.push_back(pair);
                }
            }
        }


        //Narrow Phase


        // 다이나믹 vs 스태틱(지형 등)
        for (const auto& pair : collisionPairs) {
            auto* staticCol = static_cast<Collider*>(pair.staticCollider);
            Contact contact;
            contact.rbA = pair.dynamicRb;
            contact.rbB = nullptr;
            bool isHit = CollisionSolver::CheckCollision(pair.dynamicCollider, staticCol, contact);

            std::string msg2 = "[NarrowPhase] 정밀 충돌 검사 결과 (1이면 충돌, 0이면 무시): " + std::to_string(isHit);
            LOG_DEBUG(msg2);

            if (isHit) {
                std::string msg3 = "[Solver] 💥 튕겨냅니다! 파고든 깊이: " + std::to_string(contact.penetration) +
                                   " | 법선 방향: (" + std::to_string(contact.normal.x) + ", " +
                                   std::to_string(contact.normal.y) + ", " + std::to_string(contact.normal.z) + ")";
                LOG_DEBUG(msg3);

                CollisionSolver::ResolveCollision(contact);
            }
        }

        // 다이나믹 vs 다이나믹(플레이어 간)
        for (const auto& pair : dynamicCollisionPairs) {
            Contact contact;
            contact.rbA = pair.rb1;
            contact.rbB = pair.rb2;
            if (CollisionSolver::CheckCollision(pair.c1, pair.c2, contact)) {
                CollisionSolver::ResolveCollision(contact);
            }
        }

    }
}



