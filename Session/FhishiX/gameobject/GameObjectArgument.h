//
// Created by white on 25. 5. 20.
//

#ifndef GameObjectArgument_H
#define GameObjectArgument_H
#include <vector>

#include "../../Component/Definition/ComponentArgument.h"
#include "ObjectType.h"
#include "Transform.h"
#include "../Layer.h"
#include "../ObjectTag.h"
#include "../../SessionContext.h"
#include "../../Component/Definition/ComponentFactory.h"
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
    Layers layer = Layers::Default;

    GameObjectArgument()=default;
    GameObjectArgument(const uint32_t id, uint32_t generationId):id(id),generationId(generationId){};

    uint32_t id = -1;
    uint32_t generationId = -1;
    std::string name;
    TagEnum tag = TagEnum::Untagged;
    ObjectTypeEnum type = ObjectTypeEnum::Undefined;


    [[nodiscard]] GameObject MakeHandle() const;

    // T는 ComponentArgument를 상속받아야 하고,
    // T는 Args... 인자들로 생성 가능해야 한다고 명시
    template<typename  T, typename... Args>
    requires std::constructible_from<T, Args...>
    ComponentHandle<T> AddComponent(Args&&... args);

    template<typename T>
   ComponentHandle<T> AttachComponent(ComponentHandle<T> handle) {
        if ( handle->GetGameObject() != GameObject::NullPTR())
            handle->GetGameObject()->DetachComponent(handle);
        components.push_back(std::move(handle));
        handle->SetOwner(MakeHandle());
   }
   ComponentHandleBase AddComponent(std::string typeName, std::string arg) {
        ComponentFactory::Instance().Create(typeName,MakeHandle(),arg);
    }
    template <typename T>
    ComponentHandle<T> GetComponent() {
        const size_t typeId = GetTypeId<T>();
        for (auto& comp : components) {
            if (comp.typeId == typeId) {
                return static_cast<ComponentHandle<T>&>(comp);
            }
            ComponentArgument* rawPtr = componentManagerInstance->GetRawPtr(comp.typeId, comp.entityId);
            if (rawPtr == nullptr) continue;
            if (T* castedType = dynamic_cast<T*>(rawPtr)) {
                return ComponentHandle<T>(castedType,typeId);
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
