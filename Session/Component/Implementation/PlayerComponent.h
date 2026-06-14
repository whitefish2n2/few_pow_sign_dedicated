//
// Created by white on 26. 6. 4..
//

#ifndef FPSPROJECTSERVER_PLAYERSERVERCOMPONENT_H
#define FPSPROJECTSERVER_PLAYERSERVERCOMPONENT_H
#include "../../FhishiX/gameobject/rigidBody/Rigidbody.h"
#include "../../Component/Definition/Component.h"
#include "../../FhishiX/Layer.h"

class PlayerComponent final:public Component<PlayerComponent> {
    public:
    ComponentHandle<Rigidbody> rb;

    //Weapon, HP 등등

    //플레이어 움직임
    void Move(Vector2 playerInputVector, float pitch, float yaw);;

    void Start() override;
    void FixedUpdate() override;;


    void ParseFromString(const std::string &arg) override;

    void SetCharacter(const std::string &characterId);
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
    std::string characterId = "";

    LayerMask _groundMask;
};


#endif //FPSPROJECTSERVER_PLAYERSERVERCOMPONENT_H