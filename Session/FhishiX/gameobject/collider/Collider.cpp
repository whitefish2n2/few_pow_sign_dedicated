//
// Created by white on 25. 12. 26..
//

#include "Collider.h"

#include "../GameObjectArgument.h"
///WIN64환경이면  세션에 InsertRenderer를 자동 호출함
void Collider::OnAttach() {
    ComponentArgument::OnAttach();
#ifdef _WIN64
    this->gameObject->gameSession->InsertRenderer(this->GetRenderer());
#endif
}
