//
// Created by white on 25. 5. 14.
//
#include "Player.h"

#include <utility>

void Player::SetCharacter(uint8_t charId) {
    //todo: 레전드 캐릭터 id 기반 캐릭터 스탯이고 뭐고 설정
    this->playerComponent->SetCharacter(charId);
}


Player::Player(std::string id, std::string name, std::string assignKey, uint8_t publicKey,
    playerStatus status) {
    this->userId = std::move(id);
    this->userName = std::move(name);
    this->assignKey = std::move(assignKey);
    this->publicKey = publicKey;
    this->status = std::move(status);
}



