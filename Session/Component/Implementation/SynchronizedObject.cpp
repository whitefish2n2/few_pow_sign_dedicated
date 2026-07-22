//
// Created by white on 26. 6. 2..
//
#include "../Implementation/SynchronizedObject.h"

#include "../Definition/ComponentFactory.h"

void SynchronizedObject::ParseFromString(const std::string &arg) {
    Component<SynchronizedObject>::ParseFromString(arg);
}

REGISTER_COMPONENT(SynchronizedObject)