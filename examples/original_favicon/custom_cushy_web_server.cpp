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

#include "data_custom_cushy_web_server.hpp"

#define WEB_HEADER_CACHE_CONTROL_SHORT_TIME "max-age=100, immutable"
#define WEB_HEADER_CACHE_CONTROL_LONGTIME   "max-age=31536000, immutable"
#define WEB_HEADER_CACHE_CONTROL_NO_CACHE   "no-cache"

void CustomCushyWebServer::handle_html_root(AsyncWebServerRequest *request)
{
    String html = "";
    html += "<!DOCTYPE html>";
    html += "<html lang='en'>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<link href='/general.css' rel='stylesheet' type='text/css' media='all'>";
    html += "<title>Cushy Web Server</title>";
    html += "</head>";
    html += "<body>";
    html += "<h1>Cushy Web Server</h1>";
    html += "<p><a href='/update' target='_self'>OTA</a></p>";
    html += "<p><a href='/network' target='_self'>Network</a></p>";
    html += "</body>";
    html += "</html>";
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html; charset=utf-8", html.c_str());
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_NO_CACHE);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}

bool CustomCushyWebServer::setup_server(AsyncWebServer *server)
{
    server->on("/", std::bind(&CustomCushyWebServer::handle_html_root, this, std::placeholders::_1));
    return true;
}

void CustomCushyWebServer::handle_favicon_ico(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", WEB_IMAGE_FAVICON_ICO, WEB_IMAGE_FAVICON_ICO_LEN);
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_LONGTIME);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}
