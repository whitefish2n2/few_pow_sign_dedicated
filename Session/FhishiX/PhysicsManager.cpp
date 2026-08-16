//
// Created by white on 26. 3. 16..
//
#include "PhysicsManager.h"

#include <future>
#include <vector>
#include <vector>

#include <chrono>

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
    //LOG_DEBUG("PhysicsUpdate 실행");
    DrivenPool<Rigidbody>* rigidBodies = gameSession->componentManager.get()->GetOrCreatePool<Rigidbody>();
    std::vector<DynamicProxy> dynamicProxies;
    dynamicProxies.reserve(rigidBodies->size());

    auto integrateStart = std::chrono::steady_clock::now();
    for (auto& v : *rigidBodies) {
        if (v.isActive && v.gameObject && !v.isKinematic) {
            if (v.entityId >= gameSession->physicsSystem->activeActors.size()) {
                 gameSession->physicsSystem->activeActors.resize(v.entityId + 1, PhysicsActor::NULLPTR());
            }

            auto& actor =  gameSession->physicsSystem->activeActors[v.entityId];
            if (!actor.isActive() || actor.rb.generationId != v.generationId) {
                actor.rb = v.MakeHandle();
                actor.colliders = v.gameObject->GetComponents<Collider>();
            }

            if (actor.colliders.empty()) continue;

            v.Integrate();

            for (auto& colHandle : actor.colliders) {
                if (colHandle.isNull()) continue;

                dynamicProxies.emplace_back(&v, colHandle.operator->());
            }
        }
    }
    gameSession->lastPhysicsIntegrateMicros.store(
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - integrateStart).count(),
        std::memory_order_relaxed);

    if (!dynamicProxies.empty()) {
        std::vector<CollisionPair> collisionPairs;
        std::vector<DynamicCollisionPair> dynamicCollisionPairs;
        // 보통 하나의 다이나믹 객체가 2~4개의 스태틱 객체와 겹친다고 가정하여 미리 메모리 할당
        collisionPairs.reserve(dynamicProxies.size() * 2);

        auto staticOverlapStart = std::chrono::steady_clock::now();
        for (const auto& proxy : dynamicProxies) {
            const AABB& bounds = proxy.bounds;
            Collider* myCollider = proxy.original;

            thread_local std::vector<ComponentHandle<Collider>> overlapResults;

            overlapResults.clear(); // 사이즈만 0으로 만들고 메모리는 유지
            // K-D 트리 탐색!
            gameSession->physicsSystem->tree.GetOverlaps(proxy.bounds, overlapResults);
            if (!overlapResults.empty()) {
            }
            for (auto v: overlapResults) {
                CollisionPair collisionPair;
                collisionPair.dynamicCollider = myCollider;
                collisionPair.dynamicRb = proxy.rb;
                collisionPair.staticCollider = v.operator->();
                collisionPairs.push_back(collisionPair);
            }
        }
        gameSession->lastStaticOverlapMicros.store(
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - staticOverlapStart).count(),
            std::memory_order_relaxed);
        gameSession->lastStaticPairsFound.store(static_cast<long long>(collisionPairs.size()), std::memory_order_relaxed);

        // 다이나믹-다이나믹 브로드페이즈(현재 O(n^2) 전수비교) - 그리드로 교체 예정인 구간, 격리 계측
        auto broadphaseStart = std::chrono::steady_clock::now();
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
        auto broadphaseMicros = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - broadphaseStart).count();
        gameSession->lastBroadphaseMicros.store(broadphaseMicros, std::memory_order_relaxed);
        {
            long long n = static_cast<long long>(dynamicProxies.size());
            gameSession->lastBroadphasePairsChecked.store(n > 1 ? n * (n - 1) / 2 : 0, std::memory_order_relaxed);
            gameSession->lastBroadphasePairsHit.store(static_cast<long long>(dynamicCollisionPairs.size()), std::memory_order_relaxed);
        }


        //Narrow Phase

        // 접촉쌍끼리(바닥/천장처럼 동시에 눌리는 경우 등) 서로 밀고 당기며 수렴하도록 여러 패스 반복.
        // 한 패스만 돌면 나중에 처리되는 쪽이 항상 "이겨서" 매 틱 한쪽으로 조금씩 새는(짓눌린 물체가
        // 바닥으로 꺼지는 등) 문제가 생김 — 한 접촉 내 다접점 4회 반복과 동일한 이유로 여기도 반복 필요.
        constexpr int SOLVER_PASSES = 4;
        constexpr float CONVERGED_THRESHOLD = 0.001f;   // 이 이하면 "더 보정할 게 사실상 없다"로 간주
        auto narrowPhaseStart = std::chrono::steady_clock::now();
        long long narrowPhaseStaticMicros = 0;
        long long narrowPhaseDynamicMicros = 0;
        for (int pass = 0; pass < SOLVER_PASSES; ++pass) {
            float totalPassCorrection = 0.0f;

            // 다이나믹 vs 스태틱(지형 등)
            auto staticPassStart = std::chrono::steady_clock::now();
            for (const auto& pair : collisionPairs) {
                auto* staticCol = static_cast<Collider*>(pair.staticCollider);
                Contact contact;
                contact.rbA = pair.dynamicRb;
                contact.rbB = nullptr;
                bool isHit = CollisionSolver::CheckCollision(pair.dynamicCollider, staticCol, contact);

                if (isHit) {
                    totalPassCorrection += CollisionSolver::ResolveCollision(contact);
                }
            }
            narrowPhaseStaticMicros += std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - staticPassStart).count();

            // 다이나믹 vs 다이나믹(플레이어 간)
            auto dynamicPassStart = std::chrono::steady_clock::now();
            for (const auto& pair : dynamicCollisionPairs) {
                Contact contact;
                contact.rbA = pair.rb1;
                contact.rbB = pair.rb2;
                if (CollisionSolver::CheckCollision(pair.c1, pair.c2, contact)) {
                    totalPassCorrection += CollisionSolver::ResolveCollision(contact);
                }
            }
            narrowPhaseDynamicMicros += std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - dynamicPassStart).count();

            // 이번 패스에서 실제로 보정한 양이 거의 없으면(=이미 수렴했거나 애초에 부딪힌 게 없음) 조기 종료 —
            // 가만히 서있는 흔한 케이스는 보통 1~2패스 안에 여기서 끊기고, 짓눌림처럼 진짜 다투는 경우만 4패스 다 씀.
            if (totalPassCorrection < CONVERGED_THRESHOLD) break;
        }
        gameSession->lastNarrowPhaseMicros.store(
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - narrowPhaseStart).count(),
            std::memory_order_relaxed);
        gameSession->lastNarrowPhaseStaticMicros.store(narrowPhaseStaticMicros, std::memory_order_relaxed);
        gameSession->lastNarrowPhaseDynamicMicros.store(narrowPhaseDynamicMicros, std::memory_order_relaxed);

    }
}




