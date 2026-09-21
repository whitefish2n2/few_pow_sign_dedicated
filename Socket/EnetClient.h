//
// Created by white on 25. 5. 9.
//

#ifndef ENETCLIENT_H
#define ENETCLIENT_H
#include <mutex>
#include <enet/enet.h>
#pragma push_macro("U")
#undef U
#include <concurrentqueue/concurrentqueue.h>

#include "../EnetMessage.h"
#pragma pop_macro("U")


class EnetClient {
    public:
    bool running = true;
    static EnetClient* GetInstance() {
        std::call_once(flag, []() {
            instance = new EnetClient();
        });
        return instance;
    }
    void RunClient(int port);
    void EnqueueSend(EnetMessage msg);

    private:
    inline static EnetClient* instance = nullptr;
    inline static std::once_flag flag;

    moodycamel::ConcurrentQueue<EnetMessage> sendQueue;
    void ProcessSendQueue();


    EnetClient()= default;
    EnetClient(const EnetClient&) = delete;
    EnetClient& operator=(const EnetClient&) = delete;
    void HandlePacket(ENetPeer* peer, uint8_t* data, size_t length);
    void HandleClientEvent(ENetEvent& event);

    void SendPacket(const byte *payload, size_t length, ENetPeer *peer, bool isReliable);


};


#endif //ENETCLIENT_H
