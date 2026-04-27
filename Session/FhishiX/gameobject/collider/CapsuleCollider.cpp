#include "CapsuleCollider.h"
#include <algorithm>
#include <cmath>

#include "../../../Component/Definition/ComponentFactory.h"
using namespace std;
#include "../GameObjectArgument.h"
//
// Created by white on 25. 12. 23..
//
// 1. 관성 텐서 계산 (원기둥 근사법)
Vector3 CapsuleCollider::CalculateLocalInertia(float mass) const {
    if (gameObject == GameObject::NullPTR()) return Vector3::Zero();

    // 스케일이 반영된 실제 크기
    const Vector3 scale = gameObject->transform.GetScale();
    float rScale = (direction == 0) ? (std::max)(scale.y, scale.z) :
                   (direction == 2) ? (std::max)(scale.x, scale.y) : (std::max)(scale.x, scale.z);
    float hScale = (direction == 0) ? scale.x : (direction == 2) ? scale.z : scale.y;

    float r = radius * rScale;
    float h = height * hScale;

    // 원기둥의 관성 텐서 공식 적용
    float i_main = 0.5f * mass * r * r; // 도는 축의 회전 저항
    float i_side = (1.0f / 12.0f) * mass * (3.0f * r * r + h * h); // 옆으로 넘어가는 회전 저항

    if (direction == 0) return { i_main, i_side, i_side }; // X축 캡슐
    if (direction == 2) return { i_side, i_side, i_main }; // Z축 캡슐
    return { i_side, i_main, i_side };                     // Y축 캡슐 (기본)
}
// 2. AABB 계산 (Minkowski Sum 방식을 완벽하게 최적화)
AABB CapsuleCollider::GetAABB() const {
    if (gameObject == GameObject::NullPTR()) return AABB::Empty();

    const auto& tr = gameObject->transform;
    uint32_t currentPosVer = tr.GetPosVersion();
    uint32_t currentRotVer = tr.GetRotVersion();
    uint32_t currentScaleVer = tr.GetScaleVersion();

    if (transformScaleVersion != currentScaleVer || transformRotVersion != currentRotVer) {
        transformRotVersion = currentRotVer;
        transformScaleVersion = currentScaleVer;
        transformPosVersion = currentPosVer;
        lastPosition = tr.GetPosition();

        const Vector3 pos = tr.GetPosition();
        const Vector3 scale = tr.GetScale();
        const Quaternion rot = tr.GetRotation();

        // 1) Direction(방향)에 따른 축과 스케일 계산
        float rScale, hScale;
        Vector3 localAxis = Vector3::Zero();

        if (direction == 0) { // X축으로 누운 캡슐
            rScale = (std::max)(scale.y, scale.z);
            hScale = scale.x;
            localAxis = Vector3(1, 0, 0);
        } else if (direction == 2) { // Z축으로 누운 캡슐
            rScale = (std::max)(scale.x, scale.y);
            hScale = scale.z;
            localAxis = Vector3(0, 0, 1);
        } else { // Y축으로 서 있는 캡슐 (기본)
            rScale = (std::max)(scale.x, scale.z);
            hScale = scale.y;
            localAxis = Vector3(0, 1, 0);
        }

        float scaledRadius = radius * rScale;
        float scaledTotalHeight = height * hScale;

        // 2) 내부 기둥의 중심점부터 양 끝 구의 중심까지의 거리 (Total Height - 지름)
        // 만약 Height가 지름보다 작다면 0으로 처리 (완전한 구가 됨)
        float distanceBetweenCenters = (std::max)(0.0f, scaledTotalHeight - (scaledRadius * 2.0f));
        float halfDist = distanceBetweenCenters * 0.5f;

        // 3) 로컬 상단, 하단 점 (여기에 Center 오프셋 적용)
        Vector3 scaledCenter = { center.x * scale.x, center.y * scale.y, center.z * scale.z };
        Vector3 localTop = scaledCenter + (localAxis * halfDist);
        Vector3 localBottom = scaledCenter - (localAxis * halfDist);

        // 4) 회전 및 위치 적용 (딱 2개의 점만 회전시키면 됨! 박스의 8개보다 훨씬 빠름)
        Vector3 worldTop = rot * localTop + pos;
        Vector3 worldBottom = rot * localBottom + pos;

        // 5) 두 점을 감싸는 박스를 만들고, 거기에 '반지름'만큼 일괄적으로 패딩을 준다 (Minkowski Sum)
        cachedAABB.min.x = (std::min)(worldTop.x, worldBottom.x) - scaledRadius;
        cachedAABB.min.y = (std::min)(worldTop.y, worldBottom.y) - scaledRadius;
        cachedAABB.min.z = (std::min)(worldTop.z, worldBottom.z) - scaledRadius;

        cachedAABB.max.x = (std::max)(worldTop.x, worldBottom.x) + scaledRadius;
        cachedAABB.max.y = (std::max)(worldTop.y, worldBottom.y) + scaledRadius;
        cachedAABB.max.z = (std::max)(worldTop.z, worldBottom.z) + scaledRadius;
    }
    // 위치만 바뀐 경우 초고속 갱신
    else if (transformPosVersion != currentPosVer) {
        Vector3 currentPos = tr.GetPosition();
        Vector3 delta = currentPos - lastPosition;

        cachedAABB.min += delta;
        cachedAABB.max += delta;

        transformPosVersion = currentPosVer;
        lastPosition = currentPos;
    }

    return cachedAABB;
}

void CapsuleCollider::ParseFromString(const std::string &arg) {
    std::stringstream ss(arg);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        size_t delimPos = line.find(": ");
        if (delimPos == std::string::npos) continue;

        std::string key = line.substr(0, delimPos);
        std::string val = line.substr(delimPos + 2);

        if (key == "IsTrigger") {
            this->isTrigger = (val == "1");
        }
        else if (key == "Center") {
            this->center =Vector3::ParseVector3(val);
        }
        else if (key == "Radius") {
            this->radius = std::stof(val);
        }
        else if (key == "Height") {
            this->height = std::stof(val);
        }
        else if (key == "Direction") {
            this->direction = std::stoi(val); // 0:X, 1:Y, 2:Z (Unity 기준)
        }
        else if (key == "StaticFriction") {
            this->material.staticFriction = std::stof(val);
        }
        else if (key == "DynamicFriction") {
            this->material.dynamicFriction = std::stof(val);
        }
        else if (key == "Bounciness") {
            this->material.bounciness = std::stof(val);
        }
        else if (key == "BounceCombine") {
            this->material.bounceCombine = ColliderMaterial::ParseCombineMode(val);
        }
        else if (key == "FrictionCombine") {
            this->material.frictionCombine = ColliderMaterial::ParseCombineMode(val);
        }
    }

}
#ifdef _WIN64
#include "../../Renderer.h"
Renderer CapsuleCollider::GetRenderer() {
    Renderer r{};
    if (this->gameObject == GameObject::NullPTR() || this->gameObject->id == (uint64_t)-1) {
        return r;
    }
    if (this->gameObject == GameObject::NullPTR()) return r;

    r.owner = this->gameObject;

    r.mesh = MeshManager::GetInstance()->GetUnitCapsule();

    if (r.mesh) {
        r.color = { 0.0f, 1.0f, 0.0f, 1.0f };
        r.isWireframe = true;

        // UnitCapsule: 반지름 0.5 (지름 1.0), 높이 2.0

        // 1. 가로/세로(XZ) 스케일: 반지름(radius)을 맞추려면 * 2.0 필요
        float diameterScale = this->radius * 2.0f;

        // 2. 높이(Y) 스케일: Unit 높이가 2.0이므로, 원하는 높이(height)가 되려면 / 2.0 필요
        float heightScale = this->height / 2.0f;

        r.localScale = { diameterScale, heightScale, diameterScale };

        // 오프셋
        r.localOffset = reinterpret_cast<const DirectX::XMFLOAT3&>(this->center);
    }
    else {
        r = Renderer::ErrorRenderer(this->gameObject,&this->gameObject->transform);
    }

    return r;
}
#endif

REGISTER_COMPONENT(CapsuleCollider);
