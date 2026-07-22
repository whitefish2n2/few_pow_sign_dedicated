//
// Created by white on 26. 6. 2..
//

#ifndef FPSPROJECTSERVER_SYNCHRONIZEDOBJECT_H
#define FPSPROJECTSERVER_SYNCHRONIZEDOBJECT_H
#include "../Definition/Component.h"
#include "../Definition/ComponentArgument.h"

#include "../../FhishiX/vector/Vector3.h"

class SynchronizedObject final : public Component<SynchronizedObject> {
public:
    int objectId;
    Vector3 lastSentPos = Vector3::Zero();
    Vector3 lastSentRot = Vector3::Zero();
    uint8_t resendTicks = 0;
    void ParseFromString(const std::string &arg) override;
};
#endif //FPSPROJECTSERVER_SYNCHRONIZEDOBJECT_H