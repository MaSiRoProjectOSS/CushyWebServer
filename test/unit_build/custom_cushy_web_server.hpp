/**
 * @file custom_cushy_web_server.hpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.3.0
 * @date 2023-03-28
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#ifndef CUSTOM_CUSHY_WEB_SERVER_HPP
#define CUSTOM_CUSHY_WEB_SERVER_HPP

#include <cushy_web_server.hpp>

class CustomCushyWebServer : public CushyWebServer {
public:
protected:
    bool setup_server(AsyncWebServer *server) override;

private:
    void handle_html_root(AsyncWebServerRequest *request);
    void handle_json_post(AsyncWebServerRequest *request);
};

#endif
