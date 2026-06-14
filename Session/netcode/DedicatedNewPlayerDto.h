//
// Created by user on 25. 4. 30.
//

#ifndef NEWPLAYERDTO_H
#define NEWPLAYERDTO_H
#include <string>

struct DedicatedNewPlayerDto {
    std::string id;
    std::string name;
    std::string key;
    std::string characterId;
    int team;
};
#endif //NEWPLAYERDTO_H
