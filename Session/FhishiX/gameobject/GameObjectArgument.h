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

    }
    std::vector<ComponentHandleBase> components;
    Transform transform = Transform();
    Layer layer = Layer();

    //gameobjectManager CreatObject시점에서 초기화됨
    GameSession *gameSession = nullptr;
    GameObjectArgument()=default;
    GameObjectArgument(const uint32_t id, uint32_t generationId):id(id),generationId(generationId){};

    uint32_t id = -1;
    uint32_t generationId = -1;
    std::string name;
    Tag tag;


    [[nodiscard]] GameObject MakeHandle() const;

    // T는 ComponentArgument를 상속받아야 하고,
    // T는 Args... 인자들로 생성 가능해야 한다고 명시
    template<typename  T, typename... Args>
    requires std::constructible_from<T, Args...>
    ComponentHandle<T> AddComponent(Args &&... args) {
        static_assert(std::is_base_of_v<ComponentArgument, T>, "T must derive from Component");
        ComponentHandle<T> handleT = componentManagerInstance->CreateComponentAtPool<T>(std::forward<Args>(args)...);
        handleT->SetOwner(MakeHandle());
        ComponentHandleBase componentBase = handleT;
        componentBase.typeId = handleT.getTypeId();
        components.push_back(std::move(componentBase));
        return handleT;
    }

    template<typename T>
   ComponentHandle<T> AttachComponent(ComponentHandle<T> handle) {
        if ( handle->GetGameObject() != GameObject::NullPTR())
            handle->GetGameObject()->DetachComponent(handle);
        components.push_back(std::move(handle));
        handle->SetOwner(MakeHandle());
        return handle;
   }
    void AddComponentFromString(const std::string &typeName, const std::string &arg) const;

    template <typename T>
    ComponentHandle<T> GetComponent() {
        const size_t typeId = GetTypeId<T>();
        for (auto& comp : components) {
            if (comp.typeId == typeId) {
                return static_cast<ComponentHandle<T>&>(comp);
            }
            ComponentArgument* rawPtr = componentManagerInstance->GetRawPtr(comp.typeId, comp.entityId);
            if (rawPtr == nullptr) continue;
            if ( T* castedType = dynamic_cast<T*>(rawPtr)) {
                return ComponentHandle<T>(comp.entityId,typeId);
            }
        }
        return ComponentHandle<T>::NULLPTR();
    }
template <typename T>
void DetachComponent() {
        
    }

    GameObjectArgument& operator=(const GameObjectArgument & target) = delete;

};
#endif
