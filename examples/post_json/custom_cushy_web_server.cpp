/**
 * @file custom_cushy_web_server.cpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.3.0
 * @date 2023-03-28
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#include "custom_cushy_web_server.hpp"

#define WEB_HEADER_CACHE_CONTROL_SHORT_TIME "max-age=100, immutable"
#define WEB_HEADER_CACHE_CONTROL_LONGTIME   "max-age=31536000, immutable"
#define WEB_HEADER_CACHE_CONTROL_NO_CACHE   "no-cache"

void CustomCushyWebServer::handle_json_post(AsyncWebServerRequest *request)
{
    bool result      = false;
    char buffer[255] = "";
    try {
        int len = request->args();
        if (request->args() > 0) {
            if (true == request->hasArg("id")) {
                int value = 1000 + this->to_int(request->arg("id"));
                sprintf(buffer, "{ \"value\": %d }", value);
            }
        } else {
            sprintf(buffer, "{ \"length\": %d }", len);
        }
        result = true;
    } catch (...) {
        result = false;
    }

    std::string json = this->template_json_result(result, buffer, "response data");

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json; charset=utf-8", json.c_str());
    response->addHeader("Location", String("http://") + this->get_ip().toString());
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_NO_CACHE);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}

bool CustomCushyWebServer::setup_server(AsyncWebServer *server)
{
    server->on("/post", std::bind(&CustomCushyWebServer::handle_json_post, this, std::placeholders::_1));
    return true;
}
