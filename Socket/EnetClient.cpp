//
// Created by white on 25. 5. 9.
//

#include "EnetClient.h"

#include <iostream>
#include <enet/enet.h>

#include "dto/SocketEventType.h"
#include "../Session/sessionPool/SessionManager.h"
#include "dto/AssignDto.h"
#include "dto/DefaultDto.h"

void ReturnError(ENetPeer* peer) {
    ENetPacket* packet = enet_packet_create("404", 4, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 1, packet);
}

template<typename TDto>
void RegisterPacket(SocketEventType type, uint16_t sessionKey, ENetPeer* peer, uint8_t* payload, size_t payloadLength, const uint64_t* timeStamp) {
    auto session = SessionManager::getInstance().sessions[sessionKey];
    if (session == nullptr) return;

    try {
        auto dto = std::make_shared<TDto>(TDto::Parse(payload, payloadLength));
        auto event = std::make_shared<GameEvent>();
        event->timestamp = *timeStamp;
        event->type = type;
        event->payload = dto;
        event->peer = peer;

        session->ProcessEvent(event);
    }
    catch (const std::exception& e) {
        std::cout << "[Packet Error] Type: " << type << ", Parse failed: " << e.what() << std::endl;
        const char* errorMsg = "404";
        ENetPacket* packet = enet_packet_create(errorMsg, strlen(errorMsg), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer,0,packet);
        enet_packet_destroy(packet);
    }



}

void EnetClient::HandlePacket(ENetPeer* peer, uint8_t* data, size_t length) {
    if (length < 2) {
        std::cerr << "Invalid packet length\n";
        return;
    }

    uint64_t timestamp;
    std::memcpy(&timestamp, &data[0], sizeof(uint64_t));

    uint16_t sessionKey;
    std::memcpy(&sessionKey, &data[8], sizeof(uint16_t));

    uint8_t messageType = data[10];

    uint8_t* payload = &data[11];
    size_t payloadLength = length - 3;

    try {
        switch (messageType) {
            case Assign: {
                std::string v(reinterpret_cast<char*>(payload), payloadLength);
                nlohmann::json j = nlohmann::json::parse(v);

                AssignRequestDto body;
                from_json(j, body);

                auto gameId = body.SessionId;
                auto userId = body.UserId;
                auto userKey = body.Key;

                std::shared_ptr<GameSession> currentSession;
                std::uint16_t key;

                for (auto& [k, v] : SessionManager::getInstance().sessions) {
                    if (v->sessionId == gameId) {
                        currentSession = v;
                        break;
                    }
                }

                if (currentSession == nullptr) {
                    std::cout << "session not found" << std::endl;
                    ReturnError(peer);
                    return;
                }

                key = currentSession->sessionConnectKey;

                std::cout << "player assign on session: " << currentSession->sessionId
                          << "\n player: " << userId << std::endl;

                auto p = currentSession->RegistUser(body.Key, peer);
                if (p == nullptr) {
                    std::cout << "regist user failed" << std::endl;
                    ReturnError(peer);
                    return;
                }

                AssignResponseDto response(key, p->privateKey, p->publicKey);
                nlohmann::json responseJson = response;
                auto raw = responseJson.dump();

                std::cout << raw << std::endl;

                ENetPacket* packet = enet_packet_create(raw.c_str(), raw.size(), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(peer, 1, packet);
                break;
            }

            case Move: {
                RegisterPacket<MoveDto>(Move, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }

            default: {
                RegisterPacket<DefaultDto>(SocketEventType::Default, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }
        }
    } catch (std::exception e) {
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
    enet_packet_destroy(packet);
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
        while (enet_host_service(server, &event, 1000) > 0) {
            HandleClientEvent(event);
        }
    }

    enet_host_destroy(server);
}
