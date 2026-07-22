#ifndef FPSPROJECTSERVER_INTERACTABLE_H
#define FPSPROJECTSERVER_INTERACTABLE_H
#include <string>

#include "../../Action.h"
#include "../../Game/Player.h"
#include "../Definition/Component.h"
class Interactable final : public Component<Interactable> {
public:
    static constexpr bool DO_UPDATE = false; // 업데이트 루프에서 제외 (핵심!)
    bool isInteractable = true;

    Action<Player*> onInteract;

    void ParseFromString(const std::string& arg) override {
        // 직렬화/역직렬화 로직 구현
    }

    void Interact(Player* player, bool triggerBroadcast = true) {
        if (!isInteractable) return;
        onInteract.Invoke(player);
    }
};
#endif //FPSPROJECTSERVER_INTERACTABLE_H
