//
// Created by white on 25. 5. 9.
//

#ifndef ENETCLIENT_H
#define ENETCLIENT_H
#include <mutex>
#include <enet/enet.h>

class EnetClient {
    public:
    bool running = true;
    static EnetClient* GetInstance() {
        std::call_once(flag, []() {
            instance = new EnetClient();
        });
        return instance;
    }

    private:
    static EnetClient* instance;
    static std::once_flag flag;


    EnetClient()= default;
    EnetClient(const EnetClient&) = delete;
    EnetClient& operator=(const EnetClient&) = delete;
    void HandlePacket(ENetPeer* peer, uint8_t* data, size_t length);
    void HandleClientEvent(ENetEvent& event);
    void RunClient(int port);
};

EnetClient* EnetClient::instance = nullptr;
std::once_flag EnetClient::flag;

#endif //ENETCLIENT_H
