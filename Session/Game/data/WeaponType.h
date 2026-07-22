#ifndef FPSPROJECTSERVER_WEAPONTYPE_H
#define FPSPROJECTSERVER_WEAPONTYPE_H
#pragma once
#include <string>

enum class WeaponType : int { MainWeapon = 0, SubWeapon = 1, Knife = 2, Hand = 3, Skill = 4, COUNT = 5 };

inline int WeaponTypeToSlot(const std::string& type) {
    if (type == "MainWeapon") return 0;
    if (type == "SubWeapon")  return 1;
    if (type == "Knife")      return 2;
    if (type == "Hand")       return 3;
    if (type == "Skill")      return 4;
    return -1;
}
#endif //FPSPROJECTSERVER_WEAPONTYPE_H