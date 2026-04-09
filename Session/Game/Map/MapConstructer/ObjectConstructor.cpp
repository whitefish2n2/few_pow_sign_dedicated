//
// Created by white on 26. 1. 26..
//

#include "ObjectConstructor.h"

#include <iostream>

#include "../../../GameSession.h"
#include "../../../FhishiX/gameobject/GameObjectManager.h"

GameObject ObjectConstructor::Construct(GameSession *gameSession) {
    std::cout << "[Debug] " << this->name << " 생성 시작..." << std::endl;

    // 1. 세션 자체 널 체크
    if (!gameSession || !gameSession->objectManager) {
        std::cout << "[Fatal Error] gameSession or objectManager is null!" << std::endl;
        return GameObject::NullPTR();
    }

    GameObject obj = gameSession->objectManager->CreateGameObject();

    // 2. 반환된 obj 널 체크 (핸들 내부 포인터 체크)
    if (!obj) {
        std::cout << "[Fatal Error] CreateGameObject() returned null!" << std::endl;
        return obj;
    }

    std::cout << "[Debug] GameObject 기본 할당 완료." << std::endl;

    obj->id = obj.targetId;
    obj->name = this->name;
    obj->layer = this->layer;
    obj->tag = this->tag;
    obj->transform = this->transform;

    // 3. gameSession이 제대로 연결되어 있는지 확인
    if (!obj->gameSession) {
        std::cout << "[Fatal Error] obj->gameSession is null! 초기화 누락됨!" << std::endl;
        // 임시 조치로 강제로 넣어줌 (원래는 CreateGameObject 안에서 해줘야 함)
        obj->gameSession = gameSession;
    }

    for (auto& v: this->components) {
        std::cout << "[Debug] 부착 시도 중: " << v.ComponentName << std::endl;
        v.ConstructAndAttachTo(obj); // 이 안에서 터진다면 1번이나 2번 원인
        std::cout << v.ComponentName << " 컴포넌트 부착 완료" << std::endl;

    }

    std::cout << "[Debug] " << this->name << " 생성 완전 종료." << std::endl;
    return obj;
}
