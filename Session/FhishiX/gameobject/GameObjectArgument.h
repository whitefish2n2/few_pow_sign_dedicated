//
// Created by white on 25. 5. 20.
//

#ifndef GameObjectArgument_H
#define GameObjectArgument_H
#include <string>
#include <vector>
#include <memory>

#include "ComponentArgument.h"
#include "ComponentHandle.h"
#include "ObjectType.h"
#include "Transform.h"
#include "../vector/Vector3.h"
#include "../Triangle.h"
#include "../ObjectTag.h"
#include "../AABB.h"
#include "../Layer.h"
#include "collider/Collider.h"
#include "GameObject.h"
class GameSession;

class GameObjectArgument {
protected:
    GameSession* gameSession;
    public:
    void Clear() {

    }
    std::vector<ComponentHandle> components;
    bool enablePhysics = true;
    GameObjectArgument()=default;
    GameObjectArgument(GameSession* owner) {
        gameSession = owner;
    }
    long long id;
    TagEnum tag = TagEnum::Untagged;
    ObjectTypeEnum type = ObjectTypeEnum::Undefined;

    /*
    Layers layer = Layers::Default;
    std::unique_ptr<Collider> collider;
    std::vector<Vector3> vertices;
    std::vector<Triangle> triangles = std::vector<Triangle>();
    AABB boundBox = AABB::Empty();
    Transform transform;
    */
    template<typename  T, typename... Args>
    T* AddComponent(GameObject ownerHandle, Args&&... args) {
        static_assert(std::is_base_of<ComponentArgument, T>::value, "T must derive from Component");

        auto newComponent = std::make_unique<T>(std::forward<Args>(args)...);
        newComponent.get()->setOwner(ownerHandle);
        components.push_back(std::move(newComponent));
        T* ptr = newComponent.get();
        return ptr;
    }
    template <typename T>
    T* GetComponent() {
        for (auto& comp : components) {
            // dynamic_cast로 타입 확인
            if (T* ptr = dynamic_cast<T*>(comp.get())) {
                return ptr;
            }
        }
        return nullptr;
    }

    /*
    void CalculateAABB();

    [[nodiscard]] Vector3 GetAABBCenter() const;

    [[nodiscard]] Vector3 GetAABBSize() const;

    [[nodiscard]] bool ContainsPoint(const Vector3 &point) const;

    [[nodiscard]] bool IntersectsAABB(const GameObjectArgument &other) const;

    void ExpandAABB(const Vector3 &point);

    void MergeAABB(const AABB &other);
    */
    GameObjectArgument& operator=(const GameObjectArgument & target);

};
#endif
