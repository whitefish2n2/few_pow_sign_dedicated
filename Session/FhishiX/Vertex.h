//
// Created by white on 25. 5. 12.
//

#ifndef VERTEX_H
#define VERTEX_H
#include <vector>

#include "Vector3.h"

struct Vertex
{
    std::vector<Vertex*> connected;
    Vector3 pos;
};
#endif //VERTEX_H
