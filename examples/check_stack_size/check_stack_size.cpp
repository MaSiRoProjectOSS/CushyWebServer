/**
 * @file main.cpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-22
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#include <Arduino.h>
#include <M5Atom.h>
#include <cushy_web_server.hpp>

#define WEB_HEADER_CACHE_CONTROL_SHORT_TIME "max-age=100, immutable"
#define WEB_HEADER_CACHE_CONTROL_LONGTIME   "max-age=31536000, immutable"
#define WEB_HEADER_CACHE_CONTROL_NO_CACHE   "no-cache"

class CustomCushyWebServer : public CushyWebServer {
public:
    void handle_html_root(AsyncWebServerRequest *request)
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
        response->addHeader("Location", String("http://") + this->get_ip().toString());
        response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_NO_CACHE);
        response->addHeader("X-Content-Type-Options", "nosniff");
        request->send(response);
    }

    bool setup_server(AsyncWebServer *server)
    {
        server->on("/", std::bind(&CustomCushyWebServer::handle_html_root, this, std::placeholders::_1));
        return true;
    }
};

CustomCushyWebServer cushy;

void setup()
{
    (void)M5.begin(true, false, true);
    (void)M5.dis.begin();
    (void)M5.dis.fillpix(CRGB::White);
    bool result = false;
    do {
        result = cushy.begin();
        if (false == result) {
            delay(1000);
        }
    } while (false == result);
    (void)M5.dis.fillpix(CRGB::Green);
}

void loop()
{
    (void)M5.update();
    if (true == M5.Btn.wasPressed()) {
        UBaseType_t stack_cushy = cushy.get_stack_high_water_mark();
        UBaseType_t max_cushy   = cushy.get_stack_size();

        char msg_buffer[512];
        sprintf(msg_buffer, "STACK SIZE : %d/%d", (int)stack_cushy, (int)max_cushy);
        log_i("%s", msg_buffer);
    }
    delay(1);
}
