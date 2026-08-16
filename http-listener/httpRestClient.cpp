//
// Created by user on 25. 4. 22.
//

#include <future>


#include "httpRestClient.h"

#include "../Session/sessionPool/SessionManager.h"
#include <queue>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "../Constants.h"
#include "../MonitorUtil.h"
#include "../Session/GameSession.h"

using namespace nlohmann;
void HttpRestClient::start_http_server(){
    httplib::Server svr;
    svr.Post("/makesession", [this](const httplib::Request& req, httplib::Response& res)->bool {
        try {
            std::cout << "Session creat request detected" << std::endl;
            //std::cout << req.body << std::endl;

            auto rawBody = req.body;
            json body = json::parse(rawBody);
            GameSetupBoddari initInfo;
            nlohmann::from_json(body, initInfo);
            //std::cout << "initInfo Parse Succeced" << std::endl;
            auto sessionKey = SessionManager::getInstance().makeNewSession(initInfo);
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
    svr.Post("/setcharacters", [this](const httplib::Request& req, httplib::Response& res)->bool {
        try {
            std::cout << "characters set request detected" << std::endl;
            std::cout << req.body << std::endl;

            auto rawBody = req.body;
            json body = json::parse(rawBody);
            CharacterSetDto setInfo;
            nlohmann::from_json(body, setInfo);
            SessionManager::getInstance().getSessionById(setInfo.sessionId)->SetCharacter(setInfo);
            std::cout << "set character info Parse Succeed" << std::endl;
            res.status = 201;
            return true;
        }
        catch (std::exception& e) {
            res.status = 500;
            res.set_content(e.what(), "text/plain");
            return false;
        }
    });

    svr.Get("/health", [this](const httplib::Request& req, httplib::Response& res)->bool {
        res.status = 200;
        res.set_content("OK", "text/plain");
        return true;
    });

    svr.Get("/status", [this](const httplib::Request& req, httplib::Response& res)->bool {
        try {
            ServerStatusDto status{};
            status.cpuUsagePercent = GetProcessCpuUsage();
            status.memoryUsageMB = GetProcessMemoryUsageMB();
            status.currentSessionCount = SessionManager::getInstance().getSessionCount();
            status.maxSessionCount = 100; // 임시:100개 허용

            json j = status;

            res.status = 200;
            res.set_content(j.dump(), "application/json");
            return true;
        }
        catch (std::exception& e) {
            res.status = 500;
            res.set_content(e.what(), "text/plain");
            return false;
        }
    });
    svr.listen("0.0.0.0", Consts::httpPort);
};