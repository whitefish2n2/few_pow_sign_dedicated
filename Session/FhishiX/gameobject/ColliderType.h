//
// Created by white on 25. 10. 28.
//

#ifndef OBJECTTYPE_H
#define OBJECTTYPE_H
#include <string>

enum ColliderTypeEnum {
    Mesh,
    Box,
    Capsule,
    Undefined
};
class ColliderType {
    public:
    static ColliderTypeEnum GetObjectTagFromString(const std::string &v) {
        if (v=="Mesh") return Mesh;
        if (v=="Box") return Box;
        if (v=="Capsule") return Capsule;
        return Undefined;
    }
};



#endif //OBJECTTYPE_H
