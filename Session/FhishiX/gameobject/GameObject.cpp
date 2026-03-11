#include "GameObjectManager.h"
#include "../../SessionContext.h"

GameObjectArgument *GameObject::operator->() const {
    if (IsNull(*this)) return GameObjectArgument::Empty();
    return session->objectManager->GetGameObject(*this);
}
GameObject::GameObject(GameObjectId targetId, GameObjectGenerationId gen, GameSession* ownerSession ):targetId(targetId),generationId(gen), session(ownerSession){}

GameObject &GameObject::operator=(const GameObject &target)  {
    if (this == &target)
        return *this;
    targetId = target.targetId;
    session = target.session;
    generationId = target.generationId;

    return *this;
}

bool GameObject::operator==(const GameObject &target) const {
    return targetId == target.targetId && generationId == target.generationId;
}

GameObject::operator bool() const {
    if (IsNull(*this)) return false;
    return operator->() != nullptr;
}

bool GameObject::IsNull(const GameObject &target) const {
    if (target.GetId() == -1 || target.GetGenerationId() == -1 || session == nullptr) return true;
    if (session->objectManager->GetGameObject(target) == nullptr) return true;
    return false;
}

