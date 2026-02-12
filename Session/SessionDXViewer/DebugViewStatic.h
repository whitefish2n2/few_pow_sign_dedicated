//
// Created by white on 26. 2. 6..
//
#pragma once
#include <d3d11.h>

#include "../GameSession.h"
#include "../sessionPool/SessionManager.h"
#ifdef _WIN64
#include "../FhishiX/Renderer.h"
class DebugViewStatic {
    public:
    inline static std::weak_ptr<GameSession> lookUpSession;
    inline static std::vector<std::weak_ptr<GameSession>> sessions;
    inline static int cIdx = 0;
    inline static std::vector<Renderer> renderers;
    static void ChangeUpLookUpSession() {
        UpdateSessionList();
        if (sessions.empty()) {
            cIdx = 0;
            lookUpSession.reset();
            return;
        }
        cIdx++;
        if (cIdx>=sessions.size()) {
            cIdx = 0;
        }
        lookUpSession = sessions[cIdx];
    }
    static void ChangeDownLookUpSession() {
        UpdateSessionList();
        if (sessions.empty()) {
            cIdx = 0;
            lookUpSession.reset();
            return;
        }
        cIdx--;
        if (cIdx<0) {
            cIdx = static_cast<int>(sessions.size()-1);
        }
        lookUpSession = sessions[cIdx];
    }
    static void UpdateSessionList() {
        sessions = SessionManager::getInstance().getSessionListWeak();
        if (cIdx>=sessions.size()) {
            cIdx = static_cast<int>( sessions.size()-1);
        }
    }
    inline static ID3D11RasterizerState* wireframeState = nullptr;
    inline static ID3D11RasterizerState* solidState = nullptr;
    private:

};

#endif