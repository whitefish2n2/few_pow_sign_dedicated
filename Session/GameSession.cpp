#include "GameSession.h"

#include <enet/enet.h>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <random>
#include <string>

#include "Session.h"
#include "../Constants.h"
#include "../util/util.h"
bool running = true;

GameSession::~GameSession() {
    enet_host_destroy(server);
    Log("server destroyed");
}

void GameSession::RunAsync() {
    running = true;
    std::thread([this]() {
        this->Start(); // Start는 blocking하니까 thread로 실행
    }).detach(); // 분리해서 백그라운드로 실행
}
void GameSession::Start() {
    if (server != nullptr) {
        Log("server is alady started but try to start it session id:" + sessionId);
        throw std::exception("server is already started but server trying to start server");
    }
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = Consts::port;

    server = enet_host_create(&address, 12, 2, 0, 0);
    if (!server) {
        Log("Failed to create ENet server on");
        return;
    }
    Log("server is running on port " + std::to_string(Consts::port));
    // Run server loop
    int8_t inputX,inputY;
    uint8_t messageType;
    uint8_t newPersonalId;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> distrib(0, 255);
    ENetEvent event;
    float rotX, rotY, rotZ;
    while (running) {
        while (enet_host_service(server, &event, 10) > 0) {
            std::cout<<event.type<< std::endl;
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:{
                    newPersonalId = static_cast<uint8_t>(distrib(gen));
                    const char* msg = "{d}";
                    ENetPacket* packet = enet_packet_create(msg, strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(event.peer, 0, packet);
                    enet_host_flush(server);
                    Log("Client connected on " + initInfo.gameId);
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE:{
                    std::cout << "Packet received from client.\n";
                    std::cout << "Data length: " << event.packet->dataLength << "\n";
                    std::cout << "Raw bytes: ";
                    for (size_t i = 0; i < event.packet->dataLength; ++i) {
                        printf("%02X ", event.packet->data[i]);
                    }
                    messageType = event.packet->data[0];
                    std::cout << "messageType: " << static_cast<int>(messageType) << std::endl;

                    //input Vector
                    inputX = static_cast<int8_t>(event.packet->data[1]);
                    inputY = static_cast<int8_t>(event.packet->data[2]);
                    std::cout << "inputVector: (" << static_cast<int>(inputX) << ", " << static_cast<int>(inputY) << ")" << std::endl;

                    //input Rotation
                    std::memcpy(&rotX, &event.packet->data[3], sizeof(float));
                    std::memcpy(&rotY, &event.packet->data[7], sizeof(float));
                    std::memcpy(&rotZ, &event.packet->data[11], sizeof(float));
                    std::cout << "rotEular: (" << rotX << ", " << rotY << ", " << rotZ << ")" << std::endl;
                    enet_packet_destroy(event.packet);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT:
                    Log("Client Disconnected on " + initInfo.gameId);
                status = idle;
                break;
            }
        }
    }
    enet_host_destroy(server);
}
void GameSession::Stop() {
    Log("session stopped at " + initInfo.gameId);
    running = false;
}

void GameSession::Init(std::string sessionId, GameSetupBoddari initInfo) {
    this->sessionId = sessionId;
    this->initInfo = initInfo;
    delete this->server;
    this->server = nullptr;
}

bool GameSession::reset() {
    return false;
}

void GameSession::cleanUp() {
    Stop();
    enet_host_destroy(server);
    delete this;
}

