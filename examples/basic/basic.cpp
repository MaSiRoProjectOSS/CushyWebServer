/**
 * @file main.cpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.0.1
 * @date 2023-03-22
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#include <Arduino.h>
#include <M5Atom.h>
#include <cushy_web_server.hpp>

class CustomCushyWebServer : public CushyWebServer {
public:
    void handle_html_root(AsyncWebServerRequest *request)
    {
        std::string html                 = "Hello !";
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain; charset=utf-8", html.c_str());
        response->addHeader("Cache-Control", "no-cache");
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
}
