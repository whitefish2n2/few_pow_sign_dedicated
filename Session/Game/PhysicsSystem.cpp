//
// Created by white on 25. 5. 16.
//
#include "PhysicsSystem.h"
#include "../FhishiX/gameobject/GameObjectArgument.h"



void PhysicsSystem::Init() {
}


PhysicsSystem::~PhysicsSystem() {
    for (auto& o :objects) {
        o.second->Clear();
        o.second->Clear();
    }
}

PhysicsSystem::PhysicsSystem(const PhysicsSystem & a) {
    this->objects = a.objects;
    this->type = a.type;

}

PhysicsSystem::PhysicsSystem(PhysicsSystem &&) noexcept {
}


