//
// Created by white on 25. 12. 26..
//

#include "Collider.h"
Collider::Collider(const Collider &other)  :  ComponentArgument(other) {
    this->staticObject = other.staticObject;
};

