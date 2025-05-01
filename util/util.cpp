//
// Created by user on 25. 4. 11.
//

#include "util.h"

#include <iostream>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
std::mutex coutMutex;

void Log(const std::string& message) {
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << message << std::endl;
}

std::string GetLocalIP() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        throw std::runtime_error("WSAStartup failed");
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        WSACleanup();
        throw std::runtime_error("gethostname failed");
    }
    struct hostent* host;
    addrinfo hints = {};
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo* result = nullptr;
    if (getaddrinfo(hostname, nullptr, &hints, &result) != 0) {
        WSACleanup();
        throw std::runtime_error("getaddrinfo failed");
    }
    char ipStr[INET_ADDRSTRLEN] = {};
    for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        auto* sockaddr_ipv4 = reinterpret_cast<sockaddr_in*>(ptr->ai_addr);
        if (inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, ipStr, sizeof(ipStr))) {
            break;
        }
    }
    freeaddrinfo(result);
    WSACleanup();

    if (ipStr[0] == '\0')
        throw std::runtime_error("Failed to retrieve IP address");

    return ipStr;
}

std::string GenerateUuid() {
    UUID uuid;
    if (UuidCreate(&uuid) != RPC_S_OK)
        throw std::runtime_error("Failed to create UUID");

    RPC_CSTR str = nullptr;
    if (UuidToStringA(&uuid, &str) != RPC_S_OK)
        throw std::runtime_error("Failed to convert UUID to string");

    std::string result(reinterpret_cast<char*>(str));
    RpcStringFreeA(&str);

    return result;
}
std::string wstring_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &result[0], size_needed, NULL, NULL);
    return result;
}


