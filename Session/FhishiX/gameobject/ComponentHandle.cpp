#include "ComponentHandle.h"
//
// Created by white on 25. 12. 11..
//
template<typename T>
ComponentArgument *ComponentHandle<T>::operator->() {
    return session->componentManager.GetComponentFromPool<T>(this);
}



