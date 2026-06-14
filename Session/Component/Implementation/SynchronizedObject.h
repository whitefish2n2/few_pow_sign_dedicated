//
// Created by white on 26. 6. 2..
//

#ifndef FPSPROJECTSERVER_SYNCHRONIZEDOBJECT_H
#define FPSPROJECTSERVER_SYNCHRONIZEDOBJECT_H
#include "../Definition/Component.h"
#include "../Definition/ComponentArgument.h"

class SynchronizedObject final : public Component<SynchronizedObject> {
    public:
    int objectId;
    void ParseFromString(const std::string &arg) override;
};
#endif //FPSPROJECTSERVER_SYNCHRONIZEDOBJECT_H