#pragma once

#include <cpprest/http_client.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <memory>
#include "../Session/GameSession.h"

class DedicatedServerNotifier {
public:
    static DedicatedServerNotifier& getInstance();

    void init(const std::string& clientBaseUrl);

    /// spring에 해당 서버가 실행되었음을 알려요
    /// @param key dedicate 서버 등록 key를 전달해요(spring 서버와 일치해야 함)
    /// @param ip 서버의 ip를 전달해요
    /// @param url 서버의 httpclient의 base url을 전달해요
    /// @param sessions 현재 실행중인 세션 배열을 전달해요(보통 서버 시작 시엔 빈 배열)
    void notifyDedicatedServerUp(const std::string& key, const std::string& ip,const std::string& url, const std::vector<std::shared_ptr<GameSession>>& sessions);

    /// 서버의 상태를 spring 서버에 알려요
    /// @param ip 서버의 ip를 전달해요
    /// @param sessions 현재 실행중인 세션 배열을 전달해요
    void updateServerStatus(const std::string& ip, const std::vector<std::shared_ptr<GameSession>>& sessions);
    //서버가 꺼졌음을 알려요
    void notifyDedicatedServerOff(const std::string& id);

private:
    DedicatedServerNotifier() = default;
    ~DedicatedServerNotifier() = default;

    DedicatedServerNotifier(const DedicatedServerNotifier&) = delete;
    DedicatedServerNotifier& operator=(const DedicatedServerNotifier&) = delete;

private:
    web::http::client::http_client* clientBase = nullptr;
};


