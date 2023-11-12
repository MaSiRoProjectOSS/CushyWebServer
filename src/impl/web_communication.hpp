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
    //////////////////////////////////////////////////////////////
    // Constructor and destructor
    //////////////////////////////////////////////////////////////
    WebCommunication();
    ~WebCommunication();

public:
    //////////////////////////////////////////////////////////////
    // Setup functions
    //////////////////////////////////////////////////////////////
    AsyncWebServer *get_server();
    bool setup();
    bool begin();

public:
    //////////////////////////////////////////////////////////////
    // AP settings
    //////////////////////////////////////////////////////////////
    bool reconnect_ap();
    IPAddress get_ip_address_ap();
    bool is_connected_ap(bool immediate = true);
    bool is_enable_ap();
    const char *get_ssid_ap();

public:
    //////////////////////////////////////////////////////////////
    // STA settings
    //////////////////////////////////////////////////////////////
    bool reconnect_sta();
    IPAddress get_ip_address_sta();
    String ip_to_string(IPAddress ip);
    bool is_connected_sta(bool immediate = true);
    bool is_enable_sta();
    const char *get_ssid_sta();
    void load_settings_sta(bool clear);

    int to_int(String data);

public:
    //////////////////////////////////////////////////////////////
    // template functions
    //////////////////////////////////////////////////////////////
    std::string template_json_result(bool result, std::string data = "", std::string message = "");
    String file_readString(const char *path);

    //////////////////////////////////////////////////////////////
    // Functions that are expected to be overwritten
    //////////////////////////////////////////////////////////////
    void handle_favicon_ico(AsyncWebServerRequest *request);
    void handle_not_found(AsyncWebServerRequest *request);

private:
    //////////////////////////////////////////////////////////////
    // web page handle
    //////////////////////////////////////////////////////////////
    void handle_js_ajax(AsyncWebServerRequest *request);
    void handle_css_general(AsyncWebServerRequest *request);

    void handle_network_css(AsyncWebServerRequest *request);
    void handle_network_js(AsyncWebServerRequest *request);
    void handle_network_html(AsyncWebServerRequest *request);

    void handle_network_set(AsyncWebServerRequest *request);
    void handle_network_get(AsyncWebServerRequest *request);
    void handle_network_list_get(AsyncWebServerRequest *request);
    void handle_network_list_make(AsyncWebServerRequest *request);

private:
    //////////////////////////////////////////////////////////////
    // private functions
    //////////////////////////////////////////////////////////////

private:
    WebManagerConnection _manager;
    AsyncWebServer *ctrl_server;
    bool _flag_save                    = true;
    const std::string _message_network = "The system was reconfigured."
                                         "<br />"
                                         "Please change the network connection.";
};
} // namespace Web
} // namespace MaSiRoProject
#endif
