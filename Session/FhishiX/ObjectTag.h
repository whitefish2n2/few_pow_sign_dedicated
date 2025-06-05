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
    static TagEnum GetObjectTagFromString(std::string v) {
        switch (v) {
            case "Ground": return Ground;
            case "Player": return Player;
            default: return Untagged;
        }
    }
};


#endif //OBJECTTAG_H
