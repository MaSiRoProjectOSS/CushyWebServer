/**
 * @file web_communication.hpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.0.1
 * @date 2023-03-12
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#ifndef MASIRO_PROJECT_WEB_COMMUNICATION_HPP
#define MASIRO_PROJECT_WEB_COMMUNICATION_HPP

#include "web_manager_connection.hpp"

#include <ESPAsyncWebServer.h>
#include <functional>

namespace MaSiRoProject
{
namespace Web
{

class WebCommunication {
public:
    WebCommunication();
    ~WebCommunication();

public:
    AsyncWebServer *get_server();
    bool setup();
    bool begin();
    bool reconnect_ap();
    bool reconnect_sta();

    // bool load_default(bool save);
    bool is_connected(bool immediate = true);
    std::string template_json_result(bool result, std::string data = "", std::string message = "");

public:
    bool is_enable_ap();
    IPAddress get_ip_address_ap();
    const char *get_ssid_ap();

    bool is_enable_sta();
    IPAddress get_ip_address_sta();
    const char *get_ssid_sta();
    void list_reconnect_sta();
    void load_sta_settings(bool clear);

public:
    String file_readString(const char *path);
    void handle_favicon_ico(AsyncWebServerRequest *request);

private:
    void handle_not_found(AsyncWebServerRequest *request);
    void handle_js_ajax(AsyncWebServerRequest *request);
    void handle_css_general(AsyncWebServerRequest *request);

    void handle_network_css(AsyncWebServerRequest *request);
    void handle_network_js(AsyncWebServerRequest *request);
    void handle_network_html(AsyncWebServerRequest *request);

    void handle_network_set(AsyncWebServerRequest *request);
    void handle_network_get(AsyncWebServerRequest *request);
    void handle_network_get_list(AsyncWebServerRequest *request);
    void handle_network_make_list(AsyncWebServerRequest *request);

private:
    int to_int(String data);
    bool _flag_save = true;

private:
    WebManagerConnection _manager;
    AsyncWebServer *ctrl_server;
    const std::string _message_network = "The system was reconfigured."
                                         "<br />"
                                         "Please change the network connection.";
};
} // namespace Web
} // namespace MaSiRoProject
#endif
