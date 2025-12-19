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
#include "../../Component/Definition/ComponentHandleBase.h"

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
    TagEnum tag = TagEnum::Untagged;
    ObjectTypeEnum type = ObjectTypeEnum::Undefined;


    [[nodiscard]] GameObject MakeHandle() const;

    template<typename  T, typename... Args>
    ComponentHandle<T> AddComponent(Args&&... args){
        static_assert(std::is_base_of<ComponentArgument, T>::value, "T must derive from Component");

        ComponentHandle<T> handleT = componentManagerInstance->CreateComponentAtPool<T>(std::forward<Args>(args)...);
        handleT->SetOwner(MakeHandle());
        ComponentHandleBase componentBase = handleT;
        componentBase.typeId = handleT.getTypeId();
        components.push_back(std::move(handleT));
        return handleT;
    }
    template <typename T>
    ComponentHandle<T> GetComponent() {
        for (auto& comp : components) {
            const size_t typeId = GetTypeId<T>();
            if (comp.typeId == typeId) {
                ComponentHandle<T> handle;
                handle.entityId = comp.entityId;
                handle.typeId = typeId;
                return handle;
            }
        }
        return ComponentHandle<T>::NULLPTR();
    }



    GameObjectArgument& operator=(const GameObjectArgument & target);

};
#endif
