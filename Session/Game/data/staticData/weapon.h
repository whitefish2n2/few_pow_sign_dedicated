//
// Created by white on 25. 10. 15.
//

#ifndef WEAPON_MASTER_H
#define WEAPON_MASTER_H
#include <string>

class weapon {
    public:
    std::string id;
    int magazine_max;
    int bullet_max;
    int head_damage;
    int body_damage;
    int leg_damage;
    bool available;
    int value;
};
#endif //WEAPON_MASTER_H
