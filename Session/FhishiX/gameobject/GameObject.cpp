#include "GameObjectManager.h"
#include "../../SessionContext.h"

GameObjectArgument *GameObject::operator->() const {

    return gameObjectManagerInstance->GetGameObject(*this);
}
GameObject::GameObject(GameObjectId targetId, GameObjectGenerationId gen ):targetId(targetId),generationId(gen){}

GameObject &GameObject::operator=(const GameObject &target)  {
    if (this == &target)
        return *this;
    targetId = target.targetId;
    generationId = target.generationId;
    return *this;
}

bool GameObject::operator==(const GameObject &target) const {
    return targetId == target.targetId && generationId == target.generationId;
}

GameObject::operator bool() const {
    return operator->() != nullptr;
}

bool GameObject::IsNull(const GameObject &target) {
    if (target.GetId() == -1 || target.GetGenerationId() == -1) return true;
    if (gameObjectManagerInstance->GetGameObject(target) == nullptr) return true;
    return false;
}

