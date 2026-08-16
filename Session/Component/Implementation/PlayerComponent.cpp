//
// Created by white on 26. 6. 4..
//

#include "PlayerComponent.h"

#include <cmath>

#include "../../../util/StringUtil.h"
#include "../../FhishiX/gameobject/GameObjectArgument.h"
#include "../../FhishiX/gameobject/GameObjectManager.h"
#include "../Definition/ComponentFactory.h"
#include "../../FhishiX/PhysicsManager.h"
#include "../../Game/PhysicsSystem.h"
#include "../../Game/data/CharacterRegistry.h"
#include "Weapon.h"
#include "WeaponInventory.h"
#include "../../Game/data/WeaponRegistry.h"
#include "../../FhishiX/gameobject/collider/Collider.h"


void PlayerComponent::Move(const Vector2 playerInputVector, const float pitch, const float yaw) {
    playerInputVector.normalize();
    this->inputVector = std::move(playerInputVector);
    this->aimPitch = pitch;
    this->aimYaw = yaw;
}

void PlayerComponent::Jump() {
    if (rb.isNull()) return;
    if (currentHp <= 0) return;

    Vector3 pos = gameObject->transform.GetPosition();
    bool isOnGround = gameSession->physicsSystem->CheckSphere(pos - Vector3(0, onGroundYDistance, 0), onGroundRadius, _groundMask);
    if (!isOnGround) return;

    rb.operator->()->AddImpulse(Vector3(0.0f, jumpPower, 0.0f));
}

int PlayerComponent::CalcDamage(Collider *hitCollider, Weapon *weapon) {
    if (hitCollider->gameObject != gameObject) return 0;
    if (weapon == nullptr) return 0;

    const WeaponInfo* info = weapon->GetInfo();
    if (info == nullptr) return 0;

    bool isHead = hitCollider->GetShapeType() == ColliderType::Sphere;   // 인터림: Sphere=head, 그 외=body
    return static_cast<int>(isHead ? info->headDamage : info->bodyDamage);
}

PlayerMoveSnapshot PlayerComponent::GetMoveSnapshot() {
    PlayerMoveSnapshot snap;
    snap.position = gameObject->transform.GetPosition();
    snap.rotation = Vector3(aimPitch, aimYaw, 0.0f);
    if (rb.isNull()) {
        snap.velocity = Vector3::Zero();
    } else {
        snap.velocity = rb.operator->()->linearVelocity;
    }
    return snap;
}

void PlayerComponent::Start() {
    Component<PlayerComponent>::Start();

    constexpr float kHistorySeconds = 0.5f;   // 지연보상 리와인드 상한(19c-4)과 동일
    historySize = static_cast<int>(std::ceil(kHistorySeconds / gameSession->time.DeltaTime));
    history.resize(historySize);

    rb = this->gameObject->GetComponent<Rigidbody>();
    if (rb.isNull()) {
        //std::cout<<"Player's RigidBody Is Null"<< std::endl;
        gameSession->objectManager->DeleteGameObject(gameObject);
    }
    _groundMask = gameSession->physicsSystem->layerManager.GetMask("Ground");
}

void PlayerComponent::FixedUpdate() {
    auto transform  = this->gameObject->transform;
    if (rb.isNull()) return;
    auto rbPtr = rb.operator->();

    if (currentHp <= 0) {
        rbPtr->SetVelocity(Vector3(0.0f, rbPtr->linearVelocity.y, 0.0f));
        return;
    }

    // 1. Ground Check (임시)
    bool isOnGround = true;
    isOnGround = gameSession->physicsSystem->CheckSphere(transform.GetPosition()- Vector3(0,onGroundYDistance,0), onGroundRadius, _groundMask);

    float deltaTime = gameSession->time.DeltaTime
    ;

    // 2. 현재 평면 속도 (X, Z) 추출
    Vector3 currentRbVelocity = rbPtr->linearVelocity;
    Vector3 currentPlanarVelocity = Vector3(currentRbVelocity.x, 0.0f, currentRbVelocity.z);

    // 3. 에임(Yaw) 기반 Forward, Right 벡터 계산
    float yawRad = this->aimYaw * (3.14159265f / 180.0f);
    Vector3 forward(std::sin(yawRad), 0.0f, std::cos(yawRad));
    Vector3 right(std::cos(yawRad), 0.0f, -std::sin(yawRad));

    // 4. 현재 입력(W,A,S,D) 방향 벡터 (입력이 없으면 (0,0,0)이 됨)
    Vector3 inputDir = (forward * this->inputVector.y) + (right * this->inputVector.x);
    if (inputDir.MagnitudeSq() > 1.0f) {
        inputDir.Normalize();
    }

    // C++용 MoveTowards 람다 (스텝 크기만큼 목표값을 향해 이동)
    auto MoveTowards = [](Vector3 current, Vector3 target, float maxDelta) -> Vector3 {
        Vector3 diff = target - current;
        float mag = diff.Magnitude();
        if (mag <= maxDelta || mag == 0.0f) return target;
        return current + (diff / mag) * maxDelta;
    };

    Vector3 targetVelocity = Vector3::Zero();
    float accelStep = this->acceleration * deltaTime;
    float decelStep = this->deceleration * deltaTime;

    // ✨ 5. 가속 / 감속 및 미끄러짐 로직 ✨
    if (isOnGround) {
        if (inputDir.Magnitude() > 0.1f) {
            // [입력이 있을 때] 목표 방향으로 가속 (빠릿하게)
            targetVelocity = inputDir * this->maxSpeed;
            currentPlanarVelocity = MoveTowards(currentPlanarVelocity, targetVelocity, accelStep);
        } else {
            //[입력 없을때]
            // 목표는 0(정지)이지만, Deceleration 값에 따라 서서히 줄어들면서
            // 현재 이동 중인 방향(currentPlanarVelocity)의 관성이 유지됨.
            currentPlanarVelocity = MoveTowards(currentPlanarVelocity, Vector3::Zero(), decelStep);
        }
    } else {
        // [공중 에어 스트레이핑]
        if (inputDir.Magnitude() > 0.1f) {
            // 현재 속력을 유지하면서 방향만 입력 방향으로 꺾음
            targetVelocity = inputDir * currentPlanarVelocity.Magnitude();
            currentPlanarVelocity = MoveTowards(currentPlanarVelocity, targetVelocity, accelStep);
        } else {
            // 공중에서 입력 떼면 서서히 감속
            currentPlanarVelocity = MoveTowards(currentPlanarVelocity, Vector3::Zero(), decelStep);
        }
    }

    // 6. 최종 속도 적용 (Y축 보존)
    rbPtr->SetVelocity(Vector3(currentPlanarVelocity.x, currentRbVelocity.y, currentPlanarVelocity.z));

    // 7. 본체 회전 (Yaw)
    transform.SetRotation(Quaternion::FromEuler(Vector3(0.0f, this->aimYaw, 0.0f)));

    // 히스토리 링버퍼 기록 (tick 기반 인덱스, historySize로 랩어라운드)
    if (historySize > 0) {
        int idx = gameSession->tick % historySize;
        PlayerHistoryArgument& slot = history[idx];
        slot.position = transform.GetPosition();
        slot.timestamp = std::chrono::steady_clock::now();
        lastHistoryIndex = idx;
        if (validHistorySamples < historySize) validHistorySamples++;
    }
}




void PlayerComponent::ParseFromString(const std::string &arg) {

    std::stringstream ss(arg);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        size_t delimPos = line.find(':');
        if (delimPos == std::string::npos) continue;
        std::string key = StringUtils::Trim(line.substr(0, delimPos));
        std::string val = StringUtils::Trim(line.substr(delimPos + 1));

        if (key == "MaxSpeed") {
            this->maxSpeed = std::stof(val);
        }else if (key == "Acceleration") {
            this->acceleration = std::stof(val);
        }
        else if (key == "Deceleration") {
            this->deceleration = std::stof(val);
        }
        else if (key == "MoveSpeed") {
            this->moveSpeed = std::stof(val);
        }
        else if (key == "JumpPower") {
            this->jumpPower = std::stof(val);
        }
        else if (key == "MaxHp") {
            this->maxHp = std::stoi(val);
        }
        else if (key == "OnGroundRadius") {
            this->onGroundRadius = std::stof(val);
        }
        else if (key == "OnGroundYDistance") {
            this->onGroundYDistance = std::stof(val);
        }
        else if (key == "AimOrigin") {
            this->aimOrigin = Vector3::ParseVector3(val);
        }
    }
}

void PlayerComponent::SetCharacter(uint8_t charId) {
    this->characterId = charId;
    const auto* info = CharacterRegistry::Get(charId);
    if (info) maxHp = info->maxHp;
    currentHp = maxHp;
}

void PlayerComponent::Death() {
    auto inventory = gameObject->GetComponent<WeaponInventory>();
    if (!inventory.isNull()) inventory->DropAll(publicKey);
    gameObject->SetActive(false);
}

bool PlayerComponent::TakeDamage(int amount) {
    if (currentHp <= 0) return false;   // 이미 사망 → 무시
    currentHp -= amount;
    if (currentHp <= 0) {
        currentHp = 0;
        return true;                    // 사망 전이
    }
    return false;
}


REGISTER_COMPONENT(PlayerComponent)
