//
// Created by white on 25. 5. 20.
//

#ifndef OBJECTTAG_H
#define OBJECTTAG_H
#include <string>

class ObjectTag {
    public:
    enum TagEnum {
        Ground,
        Player,
        Untagged
    };
    static TagEnum GetObjectTagFromString(const std::string &v) {
        if (v=="Ground") return TagEnum::Ground;
        if (v=="Player") return TagEnum::Player;
        else return TagEnum::Untagged;
    }
};


#endif //OBJECTTAG_H
