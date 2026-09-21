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
#include "dto/LoadingProgressDto.h"
#include "dto/GetWeaponNotifyDto.h"
#include "dto/InteractDto.h"
#include "dto/DropWeaponDto.h"
#include "dto/JumpDto.h"
#include "dto/SwapWeaponDto.h"
#include "dto/ReloadDto.h"
#include "dto/ShotDto.h"
#include "dto/HitThisDto.h"
#include "dto/HitStructureDto.h"


void ReturnError(ENetPeer* peer) {
    ENetPacket* packet = enet_packet_create("404", 4, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

///T대로 파싱해서 레지스터링
template<typename TDto>
void RegisterPacket(SocketEventType type, uint16_t sessionKey, ENetPeer* peer, uint8_t* payload, size_t payloadLength, const uint64_t* timeStamp) {
    std::shared_ptr<GameSession> session;
    {
        auto& sm = SessionManager::getInstance();
        std::shared_lock lock(sm._sessionsLock);
        auto it = sm.sessions.find(sessionKey);
        if (it == sm.sessions.end()) return;
        session = it->second;
    }
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

void EnetClient::EnqueueSend(EnetMessage msg) {
    sendQueue.enqueue(std::move(msg));
}


void EnetClient::ProcessSendQueue() {
    EnetMessage msg;
    while (sendQueue.try_dequeue(msg)) {
        for (uint8_t i = 0; i < msg.count; ++i) {
            auto& t = msg.targets[i];
            if (t.peer && t.peer->state == ENET_PEER_STATE_CONNECTED && t.peer->connectID == t.connectID)
                enet_peer_send(t.peer, 0, msg.packet);
        }
        if (msg.packet->referenceCount == 0) enet_packet_destroy(msg.packet);
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
                LOG_DEBUG("Assign Packet 왔어요");
                RegisterPacket<AssignRequestDto>(SocketEventType::Assign, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }

            case static_cast<int>(SocketEventType::Move): {
                //LOG_DEBUG("Move Packet 왔어요");
                RegisterPacket<MoveDto>(SocketEventType::Move, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }
            case static_cast<int>(SocketEventType::Progress): {
                LOG_DEBUG("Prograss 왔어요");
                RegisterPacket<LoadingProgressDto>(SocketEventType::Progress, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }

            case static_cast<int>(SocketEventType::Interact): {
                RegisterPacket<InteractDto>(SocketEventType::Interact, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }
            case static_cast<int>(SocketEventType::DropWeapon): {
                RegisterPacket<DropWeaponDto>(SocketEventType::DropWeapon, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }
            case static_cast<int>(SocketEventType::SwapWeapon): {
                RegisterPacket<SwapWeaponDto>(SocketEventType::SwapWeapon, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }
            case static_cast<int>(SocketEventType::Reload): {
                RegisterPacket<ReloadDto>(SocketEventType::Reload, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }
            case static_cast<int>(SocketEventType::Shot): {
                RegisterPacket<ShotDto>(SocketEventType::Shot, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }
            case static_cast<int>(SocketEventType::Jump): {
                RegisterPacket<JumpDto>(SocketEventType::Jump, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }
            case static_cast<int>(SocketEventType::HitThis): {
                RegisterPacket<HitThisDto>(SocketEventType::HitThis, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }
            case static_cast<int>(SocketEventType::HitStructure): {
                RegisterPacket<HitStructureDto>(SocketEventType::HitStructure, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }


            default: {
                LOG_DEBUG("다른 패킷 왔어요 type:" + std::to_string(messageType));
                RegisterPacket<DefaultDto>(SocketEventType::Default, sessionKey, peer, payload, payloadLength,&timestamp);
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return;
    }
}

// 피어별 마지막 수신 시각(ENet 스레드 전용이라 락 불필요) - DISCONNECT 시점엔 ENet이 통계를 리셋해서 직접 들고 있어야 함
static enet_uint32 lastRecvAt[ENET_PROTOCOL_MAXIMUM_PEER_ID + 1] = {};

///1초마다 CONNECTED 피어를 훑어 (리셋 전의) 진짜 무음 구간/RTT를 기록. 끊기기 전 징후 포착용
static void LogNetStat(ENetHost* host, long long maxIterMicros, size_t sendQueueApprox) {
    enet_uint32 nowMs = enet_time_get();
    int connected = 0;
    enet_uint32 worstSilence = 0;
    for (size_t i = 0; i < host->peerCount; ++i) {
        ENetPeer* p = &host->peers[i];
        if (p->state != ENET_PEER_STATE_CONNECTED) continue;
        ++connected;
        enet_uint32 silence = nowMs - p->lastReceiveTime;
        if (silence > worstSilence) worstSilence = silence;
    }
    std::cout << "[NetStat] connected=" << connected << " worstSilentMs=" << worstSilence
              << " maxLoopMs=" << maxIterMicros / 1000.0 << " sendQ~" << sendQueueApprox << std::endl;
}

void EnetClient::HandleClientEvent(ENetEvent& event) {
    switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            lastRecvAt[event.peer->incomingPeerID] = enet_time_get();
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            lastRecvAt[event.peer->incomingPeerID] = enet_time_get();
            HandlePacket(event.peer, event.packet->data, event.packet->dataLength);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT: {
            {
                // ENet이 이 이벤트를 넘기기 전에 enet_peer_reset()을 이미 불러서 rtt/loss/lastReceiveTime은 전부 초기화됨.
                // 그래서 RECEIVE 때 직접 기록해둔 시각으로 무음 구간 계산: 5000ms 이상이면 타임아웃, 짧으면 클라가 먼저 끊은 것
                auto* p = event.peer;
                auto* pl = static_cast<Player*>(p->data);
                char ip[64] = "?";
                enet_address_get_host_ip(&p->address, ip, sizeof(ip));
                std::cout << "[Disconnect] peerID=" << p->incomingPeerID
                          << " addr=" << ip << ":" << p->address.port
                          << " publicKey=" << (pl ? std::to_string(pl->publicKey) : "none")
                          << " silentMs=" << (enet_time_get() - lastRecvAt[p->incomingPeerID])
                          << " data=" << event.data << std::endl;
            }
            if (auto* player = static_cast<Player*>(event.peer->data)) {
                if (player->peer == event.peer) {   // 재접속으로 이미 새 피어에 재바인딩됐으면 보존
                    player->peer = nullptr;         // 세션 방송 즉시 차단
                    player->status.networkStatus = disconnected;
                }
            }
            event.peer->data = nullptr;             // 댕글링 방지 (무조건)
            break;
        }

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

    atexit(enet_deinitialize);

    ENetAddress address;
    ENetHost* server;

    address.host = ENET_HOST_ANY;
    address.port = port;

    server = enet_host_create(&address, 4095, 2, 0, 0);   // ENET_PROTOCOL_MAXIMUM_PEER_ID(0xFFF) - 이 한 프로세스가 받을 수 있는 절대 상한
    if (server == nullptr) {
        std::cerr << "Failed to create ENet server!" << std::endl;
        return;
    }

    std::cout << "Server started on port " << address.port << std::endl;

    ENetEvent event;
    auto statWindowStart = std::chrono::steady_clock::now();
    long long maxIterMicros = 0;   // ProcessSendQueue 호출 사이 최대 간격 = 수신 폭주 시 송신/서비스 굶김 지표
    while (running) {
        auto iterStart = std::chrono::steady_clock::now();
        ProcessSendQueue();
        while (enet_host_service(server, &event, 1) > 0) {
            HandleClientEvent(event);
        }
        auto iterEnd = std::chrono::steady_clock::now();
        auto iterMicros = std::chrono::duration_cast<std::chrono::microseconds>(iterEnd - iterStart).count();
        if (iterMicros > maxIterMicros) maxIterMicros = iterMicros;
        if (iterEnd - statWindowStart >= std::chrono::seconds(1)) {
            LogNetStat(server, maxIterMicros, sendQueue.size_approx());
            statWindowStart = iterEnd;
            maxIterMicros = 0;
        }
    }
    enet_host_destroy(server);
}
