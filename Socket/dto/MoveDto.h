//
// Created by white on 25. 5. 11.
//

#ifndef MOVEDTO_H
#define MOVEDTO_H
#include <memory>
#include <vector>
#include "../../Session/FhishiX/Vector2.h"
#include "../../Session/FhishiX/Vector3.h"

class MoveDto {
    public:
    uint64_t UserSecretKey;
    Vector2<int> InputVector;
    Vector3 RotationVector;
    static MoveDto Parse(const uint8_t* data, const size_t& size);
};



#endif //MOVEDTO_H
