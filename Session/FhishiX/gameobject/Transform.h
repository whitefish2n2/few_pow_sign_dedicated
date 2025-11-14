//
// Created by white on 25. 10. 28.
//


#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "../quaternion/Quaternion.h"
#include "../Vector/Vector3.h"

class Transform {
    public:
        Vector3 position = Vector3::Zero();
        Vector3 eularRotation = Vector3::Zero();
        Quaternion rotation = Quaternion::Identity;
        Vector3 scale = Vector3::Zero();

};
#endif //TRANSFORM_H
