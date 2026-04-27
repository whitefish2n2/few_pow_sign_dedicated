//
// Created by white on 26. 4. 22..
//

#ifndef FPSPROJECTSERVER_RAYCAST_H
#define FPSPROJECTSERVER_RAYCAST_H
#include "../../vector/Vector3.h"

class Collider;

struct Ray {
    Vector3 origin;
    Vector3 direction; // 반드시 정규화(Normalized)되어 있어야 함

    Ray(const Vector3& o, const Vector3& d) : origin(o), direction(d) {}
};

struct RaycastHit {
    Collider* collider = nullptr; // 맞은 콜라이더
    Vector3 point;                // 월드 공간에서의 충돌 지점
    Vector3 normal;               // 충돌 표면의 법선(수직) 벡터
    float distance = 0.0f;        // 광선 시작점부터 충돌 지점까지의 거리
};
#endif //FPSPROJECTSERVER_RAYCAST_H