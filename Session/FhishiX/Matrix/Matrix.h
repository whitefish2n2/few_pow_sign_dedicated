#ifndef MATRIX_H
#define MATRIX_H

#include <cmath>
#include "../Vector/Vector3.h"

struct Matrix {
    virtual ~Matrix() = default;
    virtual void Identity() = 0;
};

#endif // MATRIX_H
