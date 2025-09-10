/**
 * @file web_communication.cpp
 * @brief Webサーバーリクエスト、ネットワーク設定（AP/STA）、OTAアップデートの処理を担当します。
 * @version 0.0.1
 * @date 2023-03-12
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#include "web_communication.hpp"

#include "web_communication_data.hpp"

#include <AsyncTCP.h>
#include <SPIFFS.h>
#if FEATURES_CUSHY_WEB_SERVER_OTA
#ifndef ELEGANTOTA_USE_ASYNC_WEBSERVER
#error "Please define ELEGANTOTA_USE_ASYNC_WEBSERVER. ref https://docs.elegantota.pro/async-mode/"
#endif
#include <ElegantOTA.h>
#endif

namespace MaSiRoProject
{
namespace Web
{
#define WEB_HEADER_CACHE_CONTROL_SHORT_TIME "max-age=100, immutable"
#define WEB_HEADER_CACHE_CONTROL_LONGTIME   "max-age=31536000, immutable"
#define WEB_HEADER_CACHE_CONTROL_NO_CACHE   "no-cache"

#define STRING_BUFFER_LIMIT (255)

//////////////////////////////////////////////////////////////
// Constructor and destructor
//////////////////////////////////////////////////////////////
WebCommunication::WebCommunication()
{
    this->ctrl_server = new AsyncWebServer(SETTING_WIFI_PORT);
#if FEATURES_CUSHY_WEB_SERVER_OTA
    ElegantOTA.begin(this->ctrl_server); // Start ElegantOTA
#endif
}
WebCommunication::~WebCommunication()
{
    if (nullptr != this->ctrl_server) {
        this->ctrl_server->end();
        delete this->ctrl_server;
    }
}

//////////////////////////////////////////////////////////////
// Setup functions
//////////////////////////////////////////////////////////////
AsyncWebServer *WebCommunication::get_server()
{
    return ctrl_server;
}
bool WebCommunication::setup()
{
    bool result = true;
    if (nullptr != ctrl_server) {
        this->ctrl_server->onNotFound(std::bind(&WebCommunication::handle_not_found, this, std::placeholders::_1));

        // general data
        this->ctrl_server->on("/CushyWebServer/ajax.js", std::bind(&WebCommunication::handle_js_ajax, this, std::placeholders::_1));
        this->ctrl_server->on("/CushyWebServer/general.css", std::bind(&WebCommunication::handle_css_general, this, std::placeholders::_1));
        // change network page
        this->ctrl_server->on("/CushyWebServer/network.css", std::bind(&WebCommunication::handle_network_css, this, std::placeholders::_1));
        this->ctrl_server->on("/CushyWebServer/network.js", std::bind(&WebCommunication::handle_network_js, this, std::placeholders::_1));
        this->ctrl_server->on("/network", std::bind(&WebCommunication::handle_network_html, this, std::placeholders::_1));
        // setter/getter
        this->ctrl_server->on("/CushyWebServer/set", std::bind(&WebCommunication::handle_network_set, this, std::placeholders::_1));
        this->ctrl_server->on("/CushyWebServer/get", std::bind(&WebCommunication::handle_network_get, this, std::placeholders::_1));
        this->ctrl_server->on("/CushyWebServer/list_get", std::bind(&WebCommunication::handle_network_list_get, this, std::placeholders::_1));
        this->ctrl_server->on("/CushyWebServer/list_make", std::bind(&WebCommunication::handle_network_list_make, this, std::placeholders::_1));
    }
    return result;
}
bool WebCommunication::begin()
{
    return this->_manager.begin();
}

void WebCommunication::set_callback_reconnect_ap(ReconnectApFunction callback)
{
    this->_callback_reconnect_ap = callback;
}
void WebCommunication::set_callback_reconnect_sta(ReconnectStaFunction callback)
{
    this->_callback_reconnect_sta = callback;
}

//////////////////////////////////////////////////////////////
// AP settings
//////////////////////////////////////////////////////////////
bool WebCommunication::reconnect_ap()
{
    bool result = true;
    if (true == this->_manager.is_enable_ap()) {
        result = this->_manager.reconnect_ap(true);
        if (true == result) {
            log_i("MODE[A P] SSID[%s] Hostname[%s] IP[%s] ", //
                  this->get_ssid_ap().c_str(),
                  this->get_hostname_ap().c_str(),
                  this->get_ip_address_ap().toString().c_str());
        }
    } else {
        result = this->_manager.disconnect_ap();
    }
    return result;
}
IPAddress WebCommunication::get_ip_address_ap()
{
    return this->_manager.get_ip_address_ap();
}
bool WebCommunication::is_connected_ap(bool immediate)
{
    return this->_manager.is_connected_ap(immediate);
}
bool WebCommunication::is_enable_ap()
{
    return this->_manager.is_enable_ap();
}
String WebCommunication::get_ssid_ap()
{
    return this->_manager.get_ssid_ap();
}
String WebCommunication::get_hostname_ap()
{
    return this->_manager.get_hostname_ap();
}
bool WebCommunication::set_ap_enable(bool flag)
{
    return this->_manager.set_ap_enable(flag);
}
bool WebCommunication::save_ap_setting(bool enable, std::string ssid, std::string pass, std::string hostname)
{
    return this->_manager.save_ap_setting(enable, ssid, pass, hostname);
}

//////////////////////////////////////////////////////////////
// STA settings
//////////////////////////////////////////////////////////////
bool WebCommunication::reconnect_sta()
{
    bool result = true;
    if (true == this->_manager.is_enable_sta()) {
        result = this->_manager.reconnect_sta(true);
        if (true == result) {
            log_i("MODE[STA] SSID[%s] Hostname[%s] IP[%s] ", //
                  this->get_ssid_sta().c_str(),
                  this->get_hostname_sta().c_str(),
                  this->get_ip_address_sta().toString().c_str());
        }
    } else {
        result = this->_manager.disconnect_sta();
    }
    return result;
}
IPAddress WebCommunication::get_ip_address_sta()
{
    return this->_manager.get_ip_address_sta();
}
bool WebCommunication::is_connected_sta(bool immediate)
{
    return this->_manager.is_connected_sta(immediate);
}
bool WebCommunication::is_enable_sta()
{
    return this->_manager.is_enable_sta();
}
String WebCommunication::get_ssid_sta()
{
    return this->_manager.get_ssid_sta();
}
String WebCommunication::get_hostname_sta()
{
    return this->_manager.get_hostname_sta();
}
void WebCommunication::load_sta_settings(bool clear)
{
    this->_manager.load_sta_settings(clear);
}
bool WebCommunication::set_sta_enable(bool flag)
{
    return this->_manager.set_sta_enable(flag);
}
bool WebCommunication::save_sta_setting(bool enable, std::string ssid, std::string pass, std::string hostname, int num)
{
    return this->_manager.save_sta_setting(enable, ssid, pass, hostname, num);
}

//////////////////////////////////////////////////////////////
// template functions
//////////////////////////////////////////////////////////////
std::string WebCommunication::template_json_result(bool result, std::string data, std::string message)
{
    static bool flag_rand = false;
    static char key[5]    = "-1";
    if (false == flag_rand) {
        sprintf(key, "%d", (int)random(0, 1000));
        flag_rand = true;
    }
    std::string json = "{";
    if (true == result) {
        json.append("\"result\":\"OK\",");
    } else {
        json.append("\"result\":\"NG\",");
    }
    json.append("\"status\":{\"messages\":\"");
    if ("" != message) {
        json.append(message);
    }
    json.append("\"");
    json.append(", \"KEY\":");
    json.append(key);
    json.append("}, \"data\":");
    if ("" == data) {
        json.append("\"\"");
    } else {
        json.append(data);
    }

    json.append("}");
    return json;
}
String WebCommunication::file_readString(const char *path)
{
    return this->_manager.file_readString(path);
}

//////////////////////////////////////////////////////////////
// Functions that are expected to be overwritten
//////////////////////////////////////////////////////////////
void WebCommunication::handle_favicon_ico(AsyncWebServerRequest *request)
{
#if SETTING_DEFAULT_FAVICON
    AsyncWebServerResponse *response = request->beginResponse(200, "image/x-icon", WEB_IMAGE_FAVICON_ICO, WEB_IMAGE_FAVICON_ICO_LEN);
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_LONGTIME);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
#else
    handle_not_found(request);
#endif
}
void WebCommunication::handle_not_found(AsyncWebServerRequest *request)
{
    std::string html                 = "404 Not Found!";
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain; charset=utf-8", html.c_str());
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_NO_CACHE);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}

//////////////////////////////////////////////////////////////
// web page handle
//////////////////////////////////////////////////////////////
void WebCommunication::handle_js_ajax(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(200, "text/javascript; charset=utf-8", WEB_PAGE_AJAX_JS);
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_LONGTIME);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}
void WebCommunication::handle_css_general(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(200, "text/css; charset=utf-8", WEB_PAGE_GENERAL_CSS);
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_LONGTIME);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}

void WebCommunication::handle_network_css(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(200, "text/css; charset=utf-8", WEB_PAGE_NETWORK_CSS);
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_LONGTIME);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}
void WebCommunication::handle_network_js(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(200, "text/javascript; charset=utf-8", WEB_PAGE_NETWORK_JS);
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_LONGTIME);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}
void WebCommunication::handle_network_html(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html; charset=utf-8", WEB_PAGE_NETWORK_HTML);
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_SHORT_TIME);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}

void WebCommunication::handle_network_set(AsyncWebServerRequest *request)
{
    bool result     = false;
    String ssid     = "";
    String pass     = "";
    String hostname = "";
    int num         = -1;
    bool mode_ap    = false;
    bool state      = false;

    std::string message;
    int value = 0;
    try {
        if (request->args() > 0) {
            if (request->hasArg("ap")) {
                value   = this->to_int(request->arg("ap"));
                mode_ap = (value == 1) ? true : false;
                if (true == request->hasArg("id")) {
                    ssid = request->arg("id");
                }
                if (true == request->hasArg("pa")) {
                    pass = request->arg("pa");
                }
                if (true == request->hasArg("num")) {
                    num = this->to_int(request->arg("num"));
                }
                if (true == request->hasArg("hostname")) {
                    hostname = request->arg("hostname");
                } else {
                    hostname = String(this->_manager.get_sta_list_hostname(num).c_str());
                }
                if (true == request->hasArg("state")) {
                    state = (0 == this->to_int(request->arg("state"))) ? false : true;
                }
                result = true;
            }
        }
    } catch (...) {
        result = false;
    }
    message.append(mode_ap ? "reconnect AP" : "reconnect STA");
    message.append("[");
    message.append(ssid.c_str());
    message.append("]");
    std::string json = this->template_json_result(result, "", message);

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json; charset=utf-8", json.c_str());
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_NO_CACHE);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);

    if (true == result) {
        log_v("%s", message.c_str());
        if (true == mode_ap) {
            log_d("Set AP mode: SSID[%s] HOSTNAME[%s] %s", ssid.c_str(), hostname.c_str(), state ? "Enable" : "Disable");
            this->save_ap_setting(state, ssid.c_str(), pass.c_str(), hostname.c_str());
            if (nullptr != _callback_reconnect_ap) {
                this->_callback_reconnect_ap();
            }
        } else {
            this->_manager.set_sta_hostname(hostname.c_str());
            if (false == state) {
                num = -1;
            }
            log_d("Set STA mode: SSID[%s] HOSTNAME[%s] NUM[%d] %s", ssid.c_str(), hostname.c_str(), num, state ? "Enable" : "Disable");
            this->save_sta_setting(state, ssid.c_str(), pass.c_str(), hostname.c_str(), num);
            if (nullptr != _callback_reconnect_sta) {
                this->_callback_reconnect_sta();
            }
        }
    }
}
void WebCommunication::handle_network_get(AsyncWebServerRequest *request)
{
    std::string data = "{";
    bool mode_ap     = this->_manager.is_enable_ap();
    // AP Mode
    data.append("\"AP\": {");
    data.append("\"ssid\": \"");
    data.append(mode_ap ? this->_manager.get_ssid_ap().c_str() : SETTING_WIFI_AP_DEFAULT_SSID);
    data.append("\", \"ip\":\"");
    data.append(this->ip_to_string(this->_manager.get_ip_address_ap()).c_str());
    data.append("\", \"hostname\":\"");
    data.append(mode_ap ? this->_manager.get_hostname_ap().c_str() : SETTING_WIFI_HOSTNAME);
    data.append("\", \"enable\":");
    data.append((true == mode_ap) ? "1" : "0");
    data.append("},");

    // STA Mode
    data.append("\"STA\": {");
    data.append("\"ssid\": \"");
    data.append(this->_manager.get_ssid_sta().c_str());
    data.append("\", \"ip\":\"");
    data.append(this->ip_to_string(this->_manager.get_ip_address_sta()).c_str());
    data.append("\", \"enable\":");
    data.append((true == this->_manager.is_enable_sta()) ? "1" : "0");

    data.append(", \"selected\":");
    data.append(std::to_string(this->_manager.get_sta_list_selected()).c_str());
    data.append(", \"list\":[");
    for (int i = 0; i < SETTING_WIFI_STA_FILE_MAX; i++) {
        if (i != 0) {
            data.append(",\"");
        } else {
            data.append("\"");
        }
        data.append(this->_manager.get_sta_list_ssid(i).c_str());
        data.append("\"");
    }
    data.append("], \"hostname\":[");
    for (int i = 0; i < SETTING_WIFI_STA_FILE_MAX; i++) {
        if (i != 0) {
            data.append(",\"");
        } else {
            data.append("\"");
        }
        data.append(this->_manager.get_sta_list_hostname(i).c_str());
        data.append("\"");
    }
    data.append("]");

    data.append("}");

    //
    data.append("}");

    std::string json = this->template_json_result(true, data);

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json; charset=utf-8", json.c_str());
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_NO_CACHE);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}
void WebCommunication::handle_network_list_get(AsyncWebServerRequest *request)
{
    bool result     = true;
    bool flag_start = true;
    char buffer[STRING_BUFFER_LIMIT];
    int item_length                                      = -1;
    std::vector<WebManagerConnection::NetworkList> items = this->_manager.get_wifi_list(&item_length);

    std::string data = "{\"list\" : [";
    if (-1 == item_length) {
    } else if (-2 == item_length) {
    } else {
        for (WebManagerConnection::NetworkList item : items) {
            if (false == flag_start) {
                data.append(",");
            }
            flag_start = false;
            sprintf(buffer, "{\"name\":\"%s\" , \"power\":%d}", item.name.c_str(), item.rssi);
            data.append(buffer);
        }
    }
    data.append("]");
    sprintf(buffer, ", \"ret\" : %d}", item_length);
    data.append(buffer);

    std::string json = this->template_json_result(result, data);

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json; charset=utf-8", json.c_str());
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_NO_CACHE);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}
void WebCommunication::handle_network_list_make(AsyncWebServerRequest *request)
{
    bool result = this->_manager.make_wifi_list();

    std::string json = this->template_json_result(result);

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json; charset=utf-8", json.c_str());
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_NO_CACHE);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}

//////////////////////////////////////////////////////////////
// private functions
//////////////////////////////////////////////////////////////
int WebCommunication::to_int(String data)
{
    if (true != data.isEmpty()) {
        int value = std::stoi(data.c_str());
        return value;
    } else {
        return 0;
    }
}
String WebCommunication::ip_to_string(IPAddress ip)
{
    String res = "";
    for (int i = 0; i < 3; i++) {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}

#undef STRING_BUFFER_LIMIT

} // namespace Web
} // namespace MaSiRoProject
