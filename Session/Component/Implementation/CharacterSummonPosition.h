//
// Created by white on 26. 5. 28..
//

#ifndef FPSPROJECTSERVER_CHARACTERSUMMONPOSITION_H
#define FPSPROJECTSERVER_CHARACTERSUMMONPOSITION_H
#include "../Definition/Component.h"


class CharacterSummonPosition final: public Component<CharacterSummonPosition> {
public:
    int teamId = -1;
    void ParseFromString(const std::string &arg) override;
};


#endif //FPSPROJECTSERVER_CHARACTERSUMMONPOSITION_H