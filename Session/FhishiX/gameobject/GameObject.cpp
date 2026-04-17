#include "GameObjectManager.h"
#include "../../SessionContext.h"

GameObjectArgument *GameObject::operator->() const {
    if (IsNull(*this)) return GameObjectArgument::Empty();
    return handleSession->objectManager->GetGameObject(*this);
}
GameObject::GameObject(GameObjectId targetId, GameObjectGenerationId gen, GameSession* ownerSession ):targetId(targetId),generationId(gen), handleSession(ownerSession){}

GameObject &GameObject::operator=(const GameObject &target)  {
    if (this == &target)
        return *this;
    targetId = target.targetId;
    handleSession = target.handleSession;
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

bool GameObject::IsNull(const GameObject &target) const{
    if (target.GetId() == -1) {
        std::cout << "[IsNull FAIL] Target ID가 -1 입니다.\n";
        return true;
    }
    if (target.GetGenerationId() == -1) {
        std::cout << "[IsNull FAIL] Generation ID가 -1 입니다.\n";
        return true;
    }
    if (handleSession == nullptr) {
        std::cout << "[IsNull FAIL] Session 포인터가 nullptr 입니다.\n";
        return true;
    }

    auto* obj = handleSession->objectManager->GetGameObject(target);
    if (obj == nullptr) {
        std::cout << "[IsNull FAIL] 매니저가 객체를 찾지 못했습니다! (GetGameObject 반환값 null)\n";
        return true;
    }
    return false;
}
bool GameObject::IsNull() const{
    if (this->GetId() == -1) {
        std::cout << "[IsNull FAIL] Target ID가 -1 입니다.\n";
        return true;
    }
    if (this->GetGenerationId() == -1) {
        std::cout << "[IsNull FAIL] Generation ID가 -1 입니다.\n";
        return true;
    }
    if (handleSession == nullptr) {
        std::cout << "[IsNull FAIL] Session 포인터가 nullptr 입니다.\n";
        return true;
    }

    auto* obj = handleSession->objectManager->GetGameObject(*this);
    if (obj == nullptr) {
        std::cout << "[IsNull FAIL] 매니저가 객체를 찾지 못했습니다! (GetGameObject 반환값 null)\n";
        return true;
    }
    return false;
}

