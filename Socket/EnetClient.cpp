//
// Created by white on 25. 5. 9.
//

#include "EnetClient.h"

#include <iostream>
#include <enet/enet.h>

#include "../ObjectPool.h"
#include "dto/SocketEventType.h"
#include "../Session/sessionPool/SessionManager.h"
#include "dto/AssignDto.h"
#include "dto/DefaultDto.h"

void ReturnError(ENetPeer* peer) {
    ENetPacket* packet = enet_packet_create("404", 4, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 1, packet);
}

///T대로 파싱해서 레지스터링
template<typename TDto>
void RegisterPacket(SocketEventType type, uint16_t sessionKey, ENetPeer* peer, uint8_t* payload, size_t payloadLength, const uint64_t* timeStamp) {
    auto session = SessionManager::getInstance().sessions[sessionKey];
    if (session == nullptr) return;

    try {
        GameEvent* rawEvent = ObjectPool<GameEvent>::GetInstance().Acquire();

        GameEventPtr event(rawEvent, [](GameEvent* p) {
            ObjectPool<GameEvent>::GetInstance().Release(p);
        });
        TDto* rawDto = ObjectPool<TDto>::GetInstance().Acquire();
        rawDto->Parse(payload, payloadLength);

        std::unique_ptr<TDto, void(*)(TDto*)> uniqueDto(rawDto, [](TDto* p) {
            ObjectPool<TDto>::GetInstance().Release(p);
        });

        // variant에 소유권 이동
        event->payload = std::move(uniqueDto);
        event->timestamp = *timeStamp;
        event->type = type;
        event->peer = peer;

        session->ProcessEvent(std::move(event));
    }
    catch (const std::exception& e) {
        std::cout << "[Packet Error] Type: " << static_cast<int>(type) << ", Parse failed: " << e.what() << std::endl;
        const char* errorMsg = "404";
        ENetPacket* packet = enet_packet_create(errorMsg, strlen(errorMsg), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer,0,packet);
        //enet_packet_destroy(packet);
    }



}

void EnetClient::EnqueueSend(ENetPeer* peer, std::vector<uint8_t> payload, enet_uint32 flags) {
    std::lock_guard<std::mutex> lock(sendMutex);
    sendQueue.push({peer, std::move(payload), flags});
}

void EnetClient::ProcessSendQueue() {
    std::lock_guard<std::mutex> lock(sendMutex);
    while (!sendQueue.empty()) {
        auto& task = sendQueue.front();

        if (task.peer && task.peer->state == ENET_PEER_STATE_CONNECTED) {
            ENetPacket* packet = enet_packet_create(task.payload.data(), task.payload.size(), task.flags);
            enet_peer_send(task.peer, 0, packet);
        }
        sendQueue.pop();
    }
}

void EnetClient::HandlePacket(ENetPeer* peer, uint8_t* data, size_t length) {
    if (length < 11) {
        std::cerr << "Invalid packet length\n";
        return;
    }

    uint64_t timestamp;
    std::memcpy(&timestamp, &data[0], sizeof(uint64_t));

    uint16_t sessionKey;
    std::memcpy(&sessionKey, &data[8], sizeof(uint16_t));

    uint8_t messageType = data[10];

    uint8_t* payload = &data[11];
    size_t payloadLength = length - 11;

    try {
        switch (messageType) {
            case static_cast<int>(SocketEventType::Assign): {
                RegisterPacket<AssignRequestDto>(SocketEventType::Assign, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }

            case static_cast<int>(SocketEventType::Move): {
                RegisterPacket<MoveDto>(SocketEventType::Move, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }

            default: {
                RegisterPacket<DefaultDto>(SocketEventType::Default, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return;
    }
}

void EnetClient::HandleClientEvent(ENetEvent& event) {
    switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            std::cout << "Client connected: " << event.peer->address.host << std::endl;
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            HandlePacket(event.peer, event.packet->data, event.packet->dataLength);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            std::cout << "Client disconnected: " << event.peer->address.host << std::endl;
            break;

        default:
            break;
    }
}

void EnetClient::SendPacket(const uint8_t *payload, const size_t length, ENetPeer *peer, const bool isReliable = true) {
    ENetPacket* packet = enet_packet_create(payload, length, 0);
    enet_peer_send(peer,isReliable ? 1 : 0, packet);
    //enet_packet_destroy(packet);
}

void EnetClient::RunClient(int port) {
    if (enet_initialize() != 0) {
        std::cerr << "ENet initialization failed" << std::endl;
        return;
    }

    atexit(enet_deinitialize);

    ENetAddress address;
    ENetHost* server;

    address.host = ENET_HOST_ANY;
    address.port = port;

    server = enet_host_create(&address, 256, 2, 0, 0);
    if (server == nullptr) {
        std::cerr << "Failed to create ENet server!" << std::endl;
        return;
    }

    std::cout << "Server started on port " << address.port << std::endl;

    ENetEvent event;
    while (running) {
        ProcessSendQueue();
        while (enet_host_service(server, &event, 1) > 0) {
            HandleClientEvent(event);
        }
    }
    enet_host_destroy(server);
}
