//
// Created by white on 25. 5. 9.
//

#ifndef ENETCLIENT_H
#define ENETCLIENT_H
#include <mutex>
#include <enet/enet.h>

struct SendTask {
    ENetPeer* peer;
    std::vector<uint8_t> payload;
    enet_uint32 flags;
};

class EnetClient {
    public:
    bool running = true;
    static EnetClient* GetInstance() {
        std::call_once(flag, []() {
            instance = new EnetClient();
        });
        return instance;
    }
    void EnqueueSend(ENetPeer* peer, std::vector<uint8_t> payload, enet_uint32 flags);

    private:
    inline static EnetClient* instance = nullptr;
    inline static std::once_flag flag;

    std::queue<SendTask> sendQueue;
    std::mutex sendMutex;
    void ProcessSendQueue();


    EnetClient()= default;
    EnetClient(const EnetClient&) = delete;
    EnetClient& operator=(const EnetClient&) = delete;
    void HandlePacket(ENetPeer* peer, uint8_t* data, size_t length);
    void HandleClientEvent(ENetEvent& event);

    void SendPacket(const byte *payload, size_t length, ENetPeer *peer, bool isReliable);

    void RunClient(int port);
};


#endif //ENETCLIENT_H
