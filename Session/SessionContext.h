//
// Created by white on 25. 12. 15..
//

#ifndef FPSPROJECTSERVER_SESSIONCONTEXT_H
#define FPSPROJECTSERVER_SESSIONCONTEXT_H
#pragma once
class ComponentManager;
class GameObjectManager;
class GameSession;
/// 스레드 내부 전역 세션 인스턴스(해당 스레드 GameSession::Start()에서 초기화)
extern thread_local GameSession* gameSessionInstance;
/// 스레드 내부 전역 게임오브젝트 매니저 인스턴스(해당 스레드 GameSession::Start()에서 초기화)
extern thread_local GameObjectManager *gameObjectManagerInstance;
/// 스레드 내부 전역 컴포넌트 매니저 인스턴스(해당 스레드 GameSession::Start()에서 초기화)
extern thread_local ComponentManager* componentManagerInstance;
#endif //FPSPROJECTSERVER_SESSIONCONTEXT_H