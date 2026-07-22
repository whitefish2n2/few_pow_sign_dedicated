//
// Created by white on 25. 5. 20.
//

#ifndef GameObjectArgument_H
#define GameObjectArgument_H
#include <vector>

#include "../../Component/Definition/ComponentArgument.h"
#include "Transform.h"
#include "../Layer.h"
#include "../TagManager.h"
#include "../../GameSession.h"
#include "../../SessionContext.h"
#include "../../Component/Definition/ComponentHandle.h"
#include "../../Component/Definition/ComponentHandleBase.h"
#include "../../Component/Definition/ComponentManager.h"
class GameSession;
class GameObject;
class GameObjectArgument {
protected:
    public:
    void Clear() {
        if (gameSession && gameSession->componentManager) {
            for (auto& compBase : components) {
                if (!compBase.isNull()) {
                    gameSession->componentManager->DeleteComponentFromPoolById(compBase.typeId, compBase.entityId);
                }
            }
        }
        components.clear();
    }
    std::vector<ComponentHandleBase> components;
    Transform transform = Transform();
    Layer layer = Layer();

    //gameobjectManager CreatObject시점에서 초기화됨
    GameSession *gameSession = nullptr;
    GameObjectArgument()=default;
    GameObjectArgument(const uint32_t id, uint32_t generationId, GameSession* session = nullptr):id(id),generationId(generationId),gameSession(session){};
    GameObjectArgument(GameObjectArgument&&) noexcept = default;
    GameObjectArgument& operator=(GameObjectArgument&&) noexcept = default;

    GameObjectId id = -1;
    GameObjectGenerationId generationId = -1;
    std::string name;
    Tag tag;


    void SetActive(bool active) {
        if (!gameSession || !gameSession->componentManager) return;
        for (auto& c : components) {
            if (c.isNull()) continue;
            if (ComponentArgument* raw =
                    gameSession->componentManager->GetRawPtr(c.typeId, c.generationId, c.entityId))
                raw->isActive = active;
        }
    }

    [[nodiscard]] GameObject MakeHandle() const;


    // T는 ComponentArgument를 상속받아야 하고,
    // T는 Args... 인자들로 생성 가능해야 한다고 명시
    template<typename  T, typename... Args>
    requires std::constructible_from<T, Args...>
    ComponentHandle<T> AddComponent(Args &&... args) {
        static_assert(std::is_base_of_v<ComponentArgument, T>, "T must derive from Component");
        ComponentHandle<T> handleT = gameSession->componentManager->CreateComponentAtPool<T>(std::forward<Args>(args)...);
        handleT->SetOwner(MakeHandle());
        ComponentHandleBase componentBase = handleT;
        componentBase.typeId = handleT.getTypeId();
        components.push_back(std::move(componentBase));
        return handleT;
    }

    template<typename T>
   ComponentHandle<T> AttachComponent(ComponentHandle<T> handle) {
        if ( handle->GetGameObject() != GameObject::NullPTR())
            handle->GetGameObject()->template DetachComponent<T>();
        components.push_back(std::move(handle));
        handle->SetOwner(MakeHandle());
        return handle;
   }
    void AttachComponentBase(ComponentHandleBase handleBase) {
        components.push_back(std::move(handleBase));
        // Owner 세팅 등은 Factory나 Manager 쪽에서 처리하도록 위임
    }
    void AddComponentFromString(const std::string &typeName, const std::string &arg) const;

    template <typename T>
    ComponentHandle<T> GetComponent() {
        const size_t typeId = GetTypeId<T>();
        for (auto& comp : components) {
            if (comp.typeId == typeId) {
                return ComponentHandle<T>(comp.typeId, comp.generationId, comp.entityId, comp.componentManager);
            }
            if constexpr (!std::is_final_v<T>) {
                ComponentArgument* rawPtr = gameSession->componentManager->GetRawPtr(comp.typeId,comp.generationId,  comp.entityId);
                if (rawPtr == nullptr) continue;

                // (예: GetComponent<Collider>()를 불렀는데 현재 comp가 BoxCollider인 경우 여기서 잡힘)
                if (dynamic_cast<T*>(rawPtr)) {
                    return ComponentHandle<T>(comp.typeId, comp.generationId, comp.entityId, gameSession->componentManager.get());
                }
            }
        }
        return ComponentHandle<T>::NULLPTR();
    }
    template <typename T>
    std::vector<ComponentHandle<T>> GetComponents() {
        std::vector<ComponentHandle<T>> result;
        const size_t typeId = GetTypeId<T>();

        for (auto& comp : components) {
            if (comp.typeId == typeId) {
                result.push_back(ComponentHandle<T>(comp.typeId,comp.generationId,  comp.entityId, comp.componentManager));
                continue;
            }

            // 다형성 검사 (예: GetComponents<Collider>() 호출 시 BoxCollider, SphereCollider 모두 수집)
            if constexpr (!std::is_final_v<T>) {
                ComponentArgument* rawPtr = gameSession->componentManager->GetRawPtr(comp.typeId,comp.generationId,comp.entityId);
                if (rawPtr == nullptr) continue;

                if (dynamic_cast<T*>(rawPtr)) {
                    result.push_back(ComponentHandle<T>(comp.typeId, comp.generationId, comp.entityId, gameSession->componentManager.get()));
                }
            }
        }
        return result;
    }
    template <typename T>
        void DetachComponent() {
        const size_t typeId = GetTypeId<T>();
        for (auto it = components.begin(); it != components.end(); ) {
            if (it->typeId == typeId) {
                ComponentHandle<T> handle(it->typeId, it->generationId, it->entityId, gameSession->componentManager.get());
                gameSession->componentManager->DeleteComponentFromPool<T>(&handle);
                it = components.erase(it);
            } else {
                ComponentArgument* rawPtr = gameSession->componentManager->GetRawPtr(it->typeId,it->generationId, it->entityId);
                if (rawPtr && dynamic_cast<T*>(rawPtr)) {
                    ComponentHandle<T> handle(it->typeId, it->generationId,  it->entityId, gameSession->componentManager.get());
                    gameSession->componentManager->DeleteComponentFromPool<T>(&handle);
                    it = components.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    template <typename T>
    void DetachComponent(ComponentHandle<T> component) {
        const size_t typeId = GetTypeId<T>();
        for (auto it = components.begin(); it != components.end(); ) {
            if (it->typeId == typeId && it->entityId == component.entityId) {
                gameSession->componentManager->DeleteComponentFromPool<T>(&component); // 포인터로 넘김
                it = components.erase(it);
                return; // 찾았으니 종료
            }
            ++it;
        }
    }

    template <typename T>
    bool hasThisComponent() {
        auto typeId = GetTypeId<T>();
        for (auto& comp : components) {
            if (comp.typeId == typeId) {
                return true;
            }
        }
        return false;
    }

    GameObjectArgument& operator=(const GameObjectArgument & target) = delete;
    GameObjectArgument(const GameObjectArgument&) = delete;
    static GameObjectArgument *Empty() {
        static GameObjectArgument empty = {};
        return &empty;
    }

};
#endif
