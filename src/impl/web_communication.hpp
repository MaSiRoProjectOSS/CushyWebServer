/**
 * @file web_communication.hpp
 * @brief AP/STAネットワーク設定やWebページ処理を提供します。
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
    // Callback
    //////////////////////////////////////////////////////////////
#if CALLBACK_STYLE_USING
    using ReconnectApFunction  = void (*)();
    using ReconnectStaFunction = void (*)();
#else
    typedef std::function<void(void)> ReconnectApFunction;
    typedef std::function<void(void)> ReconnectStaFunction;
#endif

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
    void set_callback_reconnect_ap(ReconnectApFunction callback);
    void set_callback_reconnect_sta(ReconnectStaFunction callback);

public:
    //////////////////////////////////////////////////////////////
    // AP settings
    //////////////////////////////////////////////////////////////
    bool reconnect_ap();
    IPAddress get_ip_address_ap();
    bool is_connected_ap(bool immediate = true);
    bool is_enable_ap();
    String get_ssid_ap();
    String get_hostname_ap();

    bool set_ap_enable(bool flag);
    bool save_ap_setting(bool enable, std::string ssid, std::string pass, std::string hostname);

public:
    //////////////////////////////////////////////////////////////
    // STA settings
    //////////////////////////////////////////////////////////////
    bool reconnect_sta();
    IPAddress get_ip_address_sta();
    bool is_connected_sta(bool immediate = true);
    bool is_enable_sta();
    String get_ssid_sta();
    String get_hostname_sta();

    void load_sta_settings(bool clear);
    bool set_sta_enable(bool flag);
    bool save_sta_setting(bool enable, std::string ssid, std::string pass, std::string hostname, int num);

public:
    //////////////////////////////////////////////////////////////
    // Convert functions
    //////////////////////////////////////////////////////////////
    String ip_to_string(IPAddress ip);
    int to_int(String data);
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
    ReconnectApFunction _callback_reconnect_ap;
    ReconnectStaFunction _callback_reconnect_sta;

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
