//
// Created by user on 25. 4. 22.
//

#include <future>


#include "httpRestClient.h"

#include "../Session/sessionPool/SessionManager.h""
#include <queue>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include "../Session/GameSession.h"

using namespace nlohmann;
void HttpRestClient::start_http_server(){
    httplib::Server svr;
    /*
    gameserverBoddari:
    val gameId:String,
    val players:List<PlayerDto>,
    val gameMode:GameMode,
    val map: String,
     */
    svr.Post("/makesession", [this](const httplib::Request& req, httplib::Response& res)->bool {
        try {
            std::cout << "Session creat request detected" << std::endl;
            std::cout << req.body << std::endl;

            auto rawBody = req.body;
            json body = json::parse(rawBody);
            GameSetupBoddari initInfo;
            nlohmann::from_json(body, initInfo);
            std::cout << "initInfo Parse Succeced" << std::endl;
            auto sessionKey = SessionManager::getInstance().makeNewSession(initInfo);
            res.body =std::to_string((int)sessionKey);
            res.status = 201;
            res.set_content(std::to_string(sessionKey), "application/json");
            std::cout << "Session created. ID: " << initInfo.gameId <<", session key:" << std::to_string((int)sessionKey)<< std::endl;
            return true;
        }
        catch (std::exception& e) {
            res.status = 500;
            res.set_content(e.what(), "text/plain");
            return false;
        }

    });

    svr.listen("0.0.0.0", 8888);
};