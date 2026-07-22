//
// Created by white on 26. 6. 4..
//

#ifndef FPSPROJECTSERVER_PLAYERSERVERCOMPONENT_H
#define FPSPROJECTSERVER_PLAYERSERVERCOMPONENT_H
#include "../../FhishiX/gameobject/rigidBody/Rigidbody.h"
#include "../../Component/Definition/Component.h"
#include "../../FhishiX/Layer.h"
class Weapon;
class Collider;

struct PlayerMoveSnapshot {
    Vector3 position;
    Vector3 rotation;   // x=pitch, y=yaw
    Vector3 velocity;
};
struct PlayerHistoryArgument {
    Vector3 position;
    std::chrono::steady_clock::time_point timestamp;
};

class PlayerComponent final:public Component<PlayerComponent> {
    public:
    ComponentHandle<Rigidbody> rb;
    uint8_t publicKey = 0;

    Vector3 aimOrigin = Vector3(0.0f, 1.74f, 0.0f);

    std::vector<PlayerHistoryArgument> history;
    int historySize = 0;   // Start()에서 time_step 비례로 계산되는 링버퍼 크기 (history.size()와 동일)
    int lastHistoryIndex = 0;
    int validHistorySamples = 0;

    //플레이어 움직임
    void Move(Vector2 playerInputVector, float pitch, float yaw);

    void Jump();

    int GetCurrentHp() {
        return currentHp;
    };

    int CalcDamage(Collider* hitCollider, Weapon* weapon);
    bool TakeDamage(int amount);                          // 사망 전이 시 true
    void Death();                                          // 비활성화 + 무기 전부 드롭
    void ResetLife() { currentHp = maxHp; }               // 스폰/리스폰 HP 복구
    [[nodiscard]] bool IsAlive() const { return currentHp > 0; }  //  피탄/판정용

    PlayerMoveSnapshot GetMoveSnapshot();
    void Start() override;
    void FixedUpdate() override;


    void ParseFromString(const std::string &arg) override;

    void SetCharacter(uint8_t charId);
private:
    Vector2 inputVector = {};
    Vector3 moveVector = {};
    float aimPitch = 0;
    float aimYaw = 0;
    float maxSpeed = 0;
    float acceleration = 0;
    float deceleration = 0;
    float moveSpeed = 0;
    float jumpPower = 0;
    float onGroundRadius = 0;
    float onGroundYDistance = 0;
    int maxHp = 0;
    int currentHp = 0;
    std::string characterId = "";

    LayerMask _groundMask;
};


#endif //FPSPROJECTSERVER_PLAYERSERVERCOMPONENT_H