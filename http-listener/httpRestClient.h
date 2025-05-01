//
// Created by user on 25. 4. 22.
//

#ifndef HTTPRESTCLIENT_H
#define HTTPRESTCLIENT_H

#include <queue>
#include <stack>
#include <future>
#include "../Session/netcode/newPlayerDto.h"
#include "../third-party/httplib.h"
#include "../Session/netcode/SessionNetworkDto.h"
class HttpRestClient {
public:
    struct SessionRequest {
        GameSetupBoddari initInfo;
        std::promise<std::string> promise;
    };

    static HttpRestClient* getInstance() {
        static HttpRestClient instance;
        return &instance;
    }
    std::mutex queueMutex;
    std::condition_variable queueCV;
    std::queue<std::shared_ptr<SessionRequest>> requestQueue;
    httplib::Server svr;
    void start_http_server();
};




#endif //HTTPRESTCLIENT_H
