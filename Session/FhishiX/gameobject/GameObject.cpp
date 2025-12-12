#include "GameObjectArgument.h"
#include "GameObjectManager.h"
#include "../../GameSession.h"
#include "../../Game/MapManager.h"

GameObjectArgument *GameObject::operator->() const {
    return gameSession->objectManager.GetGameObject(this);
}
GameObject::GameObject(GameSession* owner) {
    gameSession = owner;
}
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

