//
// Created by white on 25. 5. 23.
//

#ifndef PROPERTY_H
#define PROPERTY_H
#include <string>

#include "../FhishiX/GameObject.h"


class Property {
public:
    virtual ~Property() {}
    virtual std::string GetName() const = 0;
    virtual std::string Attach(GameObject* obj) {

    };
};



#endif //PROPERTY_H
