//
// Created by white on 25. 5. 14.
//
#include "Player.h"

#include <utility>

void Player::SetCharacter(const std::string characterId) {
    //todo: 레전드 캐릭터 id 기반 캐릭터 스탯이고 뭐고 설정
}

void Player::Move(const Vector2 inputVector)
{
    status.position += inputVector;
    //레전드 벨로시티연산으로 변환할 필요가있음
}
void Player::Rotate(const Vector3 r)
{
    this->status.rotation = r;
}
void Player::Jump()
{

}

Player::Player(std::string id, std::string name, std::string assignKey, uint64_t privateKey, uint8_t publicKey,
    player_status status) {
    this->userId = std::move(id);
    this->userName = std::move(name);
    this->assignKey = std::move(assignKey);
    this->privateKey = privateKey;
    this->publicKey = publicKey;
    this->status = std::move(status);
}



