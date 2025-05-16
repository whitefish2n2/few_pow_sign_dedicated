#include "./DedicateServerNotifier.h"
#include "../server-status/ServerStat.h"
#include "../Session/netcode/SessionNetworkDto.h"
#include "../util/util.h"
#include <iostream>

using namespace web;
using namespace web::http;
using namespace web::http::client;

DedicatedServerNotifier& DedicatedServerNotifier::getInstance() {
    static DedicatedServerNotifier instance;
    return instance;
}

void DedicatedServerNotifier::init(const std::string& clientBaseUrl) {
    std::cout<<"Initializing DedicatedServerNotifier with base url:"<<clientBaseUrl<<std::endl;
    if (clientBase != nullptr) {
        delete clientBase;
    }
    auto baseUrl = U("http://") + utility::conversions::to_string_t(clientBaseUrl);
    clientBase = new http_client(baseUrl);
    std::cout<<"Init Notifier successfully!"<<std::endl;
}

void DedicatedServerNotifier::notifyDedicatedServerUp(const std::string& key, const std::string& ip,const std::string& url, const std::vector<std::shared_ptr<GameSession>>& sessions) {
    if (!clientBase) {
        std::cout << "DedicatedServerNotifier not initialized!" << std::endl;
        return;
    }

    std::cout << "notifier server setup started!"<< std::endl;

    nlohmann::json body;
    body["key"] = key;
    body["ip"] = ip;
    body["url"] = url;
    body["sessions"] = nlohmann::json::array();

    for (const auto& s : sessions) {
        if (s == nullptr) continue;
        auto session = getGameSessionDto(*s);
        nlohmann::json j;
        to_json(j, session);
        body["sessions"].push_back(j);
    }

    try {
        std::string jsonText = body.dump();
        std::cout << "Dump succeeded: " << jsonText << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "DUMP FAILED: " << e.what() << std::endl;
    }

    std::string requestBody = body.dump();
    std::cout << "Request body: " << requestBody << std::endl;

    try {

        http_request request(methods::POST);

        request.set_request_uri(U("/dedicated/creatededicated"));

        request.headers().set_content_type(U("application/json"));

        request.set_body(utility::conversions::to_string_t(requestBody));

        clientBase->request(request)
            .then([](http_response response) -> Concurrency::task<std::wstring> {
                if (response.status_code() == status_codes::OK) {
                    return response.extract_string();
                } else {
                    std::cout << "Registration failed. Status: " << response.status_code() << std::endl;
                    return pplx::task_from_result(utility::string_t{});
                }
            })
            .then([](utility::string_t jsonRespStr) {
                if (!jsonRespStr.empty()) {
                    std::string jsonRespUtf8 = utility::conversions::to_utf8string(jsonRespStr);
                    auto jsonResp = nlohmann::json::parse(jsonRespUtf8);

                    std::string serverId = jsonResp["serverId"];
                    int statusCode = jsonResp["httpStatusCode"];

                    ServerStat::ServerId = serverId;
                    std::cout << "Dedicated server registered. ID: " << serverId << ", Status: " << statusCode << std::endl;
                } else {
                    std::cout << "No JSON returned." << std::endl;
                }
            }).wait();
    }
    catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

void DedicatedServerNotifier::updateServerStatus(const std::string& ip, const std::vector<std::shared_ptr<GameSession>>& sessions) {
    if (!clientBase) {
        std::cout << "DedicatedServerNotifier not initialized!" << std::endl;
        return;
    }

    nlohmann::json body;
    body["ip"] = ip;

    try {
        http_request request(methods::POST);
        request.set_request_uri(U("/dedicated/updatestatus"));
        request.headers().set_content_type(U("application/json"));
        request.set_body(utility::conversions::to_string_t(body.dump()));

        clientBase->request(request)
            .then([](const http_response &response) {
                if (response.status_code() == status_codes::OK) {
                    std::cout << "Server status updated successfully." << std::endl;
                } else {
                    std::cout << "Status update failed. Status: " << response.status_code() << std::endl;
                }
            }).wait();
    }
    catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

void DedicatedServerNotifier::notifyDedicatedServerOff(const std::string& id) {
    if (!clientBase) {
        std::cout << "DedicatedServerNotifier not initialized!" << std::endl;
        return;
    }

    nlohmann::json body;
    body["id"] = id;

    try {
        http_request request(methods::POST);
        request.set_request_uri(U("/dedicated/deletededicated"));
        request.headers().set_content_type(U("application/json"));
        request.set_body(utility::conversions::to_string_t(body.dump()));

        clientBase->request(request)
            .then([](http_response response) {
                if (response.status_code() == status_codes::OK) {
                    std::cout << "Server unregistered successfully." << std::endl;
                } else {
                    std::cout << "Unregistration failed. Status: " << response.status_code() << std::endl;
                }
            }).wait();
    }
    catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}


