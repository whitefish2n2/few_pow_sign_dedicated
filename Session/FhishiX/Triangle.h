//
// Created by white on 25. 5. 20.
//

#ifndef TRIANGLE_H
#define TRIANGLE_H
struct Triangle {
    uint32_t v0{0}, v1{0}, v2{0};
    Triangle(int v0, int v1, int v2) : v0(v0), v1(v1), v2(v2) {}
    Triangle(uint32_t v0, uint32_t v1, uint32_t v2) : v0(v0), v1(v1), v2(v2) {}
    Triangle()= default;
};
#endif //TRIANGLE_H
