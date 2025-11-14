//
// Created by white on 25. 5. 20.
//

#ifndef OBJECTTAG_H
#define OBJECTTAG_H
#include <string>
enum TagEnum {
    Ground,
    PlayerTag,
    Untagged
};
class ObjectTag {
    public:
    static TagEnum GetObjectTagFromString(const std::string &v) {
        if (v=="Ground") return TagEnum::Ground;
        if (v=="Player") return TagEnum::PlayerTag;
        else return TagEnum::Untagged;
    }
};


#endif //OBJECTTAG_H
