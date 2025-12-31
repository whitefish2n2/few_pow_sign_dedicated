//
// Created by white on 25. 12. 12..
//
#include "ComponentArgument.h"

ComponentArgument::ComponentArgument(const ComponentArgument &other) :gameObject(GameObject::NullPTR()), entityId(-1) {
    //entityId와 gameObject는 복사 금지
}

