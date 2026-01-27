//
// Created by white on 26. 1. 26..
//

#include "ObjectConstructor.h"

#include "../../../GameSession.h"
#include "../../../FhishiX/gameobject/GameObjectManager.h"

GameObject ObjectConstructor::Construct(GameSession *gameSession) {
    GameObject obj = gameSession->objectManager->CreateGameObject();
    obj->name = this->name;
    obj->layer = this->layer;
    obj->tag = this->tag;
    for (auto& v: this->components) {
        v.ConstructAndAttachTo(obj);
    }
    return obj;
}
