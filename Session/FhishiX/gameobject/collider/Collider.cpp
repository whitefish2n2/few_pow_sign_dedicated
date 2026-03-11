//
// Created by white on 25. 12. 26..
//

#include "Collider.h"

#include "../GameObjectArgument.h"
///WIN64환경이면  세션에 InsertRenderer를 자동 호출함
void Collider::OnAttach() {
    ComponentArgument::OnAttach();
    std::cout<<"BoxCollider OnAttach Try . . ."<<std::endl;
    if (this->gameObject.IsNull(this->gameObject)) return;
    auto rawObj = this->gameObject.operator->();
    if (rawObj == nullptr || rawObj->gameSession == nullptr) return;
    std::cout<<"Complete."<<std::endl;
#ifdef _WIN64
    rendererIndex = rawObj->gameSession->InsertRenderer(this->GetRenderer());
    std::cout << "렌더러 Insert:"<<this->gameObject->name << std::endl;
#endif
}
