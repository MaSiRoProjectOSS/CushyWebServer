/**
 * @file web_communication.cpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
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
#if CUSHY_WEB_SERVER_OTA
#include <AsyncElegantOTA.h>
#endif

namespace MaSiRoProject
{
namespace Web
{
#define WEB_HEADER_CACHE_CONTROL_SHORT_TIME "max-age=100, immutable"
#define WEB_HEADER_CACHE_CONTROL_LONGTIME   "max-age=31536000, immutable"
#define WEB_HEADER_CACHE_CONTROL_NO_CACHE   "no-cache"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

String WebCommunication::file_readString(const char *path)
{
    String word;
    word.clear();
    if (true == SPIFFS.begin()) {
        File file   = SPIFFS.open(path, FILE_READ);
        size_t size = file.size();
        word        = file.readString();
        file.close();
        SPIFFS.end();
    }
    return word;
}

void WebCommunication::handle_not_found(AsyncWebServerRequest *request)
{
    std::string html                 = "404 Not Found!";
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain; charset=utf-8", html.c_str());
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_NO_CACHE);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}
void WebCommunication::handle_favicon_ico(AsyncWebServerRequest *request)
{
#if SETTING_DEFAULT_FAVICON
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", WEB_IMAGE_FAVICON_ICO, WEB_IMAGE_FAVICON_ICO_LEN);
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_LONGTIME);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
#else
    handle_not_found(request);
#endif
}
void WebCommunication::handle_js_ajax(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript; charset=utf-8", WEB_PAGE_AJAX_JS);
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_LONGTIME);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}

void WebCommunication::handle_css_general(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css; charset=utf-8", WEB_PAGE_GENERAL_CSS);
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_LONGTIME);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}

void WebCommunication::handle_network_css(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css; charset=utf-8", WEB_PAGE_NETWORK_CSS);
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_LONGTIME);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}
void WebCommunication::handle_network_js(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript; charset=utf-8", WEB_PAGE_NETWORK_JS);
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_LONGTIME);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}

void WebCommunication::handle_network_html(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html; charset=utf-8", WEB_PAGE_NETWORK_HTML);
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_SHORT_TIME);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}

void WebCommunication::handle_network_set(AsyncWebServerRequest *request)
{
    bool result  = false;
    String ssid  = "";
    String pass  = "";
    bool mode_ap = this->_manager.is_enable_ap();

    try {
        if (request->args() > 0) {
            if (request->hasArg("ap")) {
                int value = this->to_int(request->arg("ap"));
                if (value == 1) {
                    mode_ap = true;
                }
                if (request->hasArg("id")) {
                    ssid = request->arg("id");
                    if (request->hasArg("pa")) {
                        pass   = request->arg("pa");
                        result = true;
                    }
                }
            }
        }
    } catch (...) {
        result = false;
    }
    std::string json = this->template_json_result(result);

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json; charset=utf-8", json.c_str());
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_NO_CACHE);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);

    if (true == result) {
        if (true == mode_ap) {
            this->_flag_save = this->_manager.reconnect_ap(ssid.c_str(), pass.c_str(), this->_flag_save);
        } else {
            this->_flag_save = this->_manager.reconnect_sta(ssid.c_str(), pass.c_str(), this->_flag_save);
        }
    }
}
void WebCommunication::handle_network_make_list(AsyncWebServerRequest *request)
{
    bool result = this->_manager.make_wifi_list();

    std::string json = this->template_json_result(result);

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json; charset=utf-8", json.c_str());
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_NO_CACHE);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}
void WebCommunication::handle_network_get_list(AsyncWebServerRequest *request)
{
    bool result     = true;
    bool flag_start = true;
    char buffer[255];
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
void WebCommunication::handle_network_get(AsyncWebServerRequest *request)
{
    std::string data = "{";
    data.append("\"AP\": {");
    data.append("\"default\": \"");
    data.append(SETTING_WIFI_AP_DEFAULT_SSID);
    data.append("\", \"name\": \"");
    data.append(this->_manager.get_ssid_ap());
    data.append("\", \"enable\":");
    data.append((true == this->_manager.is_enable_ap()) ? "1" : "0");
    data.append("},");
    data.append("\"STA\": {");
    data.append("\"default\": \"");
    data.append(SETTING_WIFI_STA_DEFAULT_SSID);
    data.append("\", \"name\": \"");
    data.append(this->_manager.get_ssid_sta());
    data.append("\", \"enable\":");
    data.append((true == this->_manager.is_enable_sta()) ? "1" : "0");
    data.append("}");
    data.append("}");

    std::string json = this->template_json_result(true, data);

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json; charset=utf-8", json.c_str());
    response->addHeader("Cache-Control", WEB_HEADER_CACHE_CONTROL_NO_CACHE);
    response->addHeader("X-Content-Type-Options", "nosniff");
    request->send(response);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
bool WebCommunication::setup()
{
    bool result = true;
    if (nullptr != ctrl_server) {
        this->ctrl_server->onNotFound(std::bind(&WebCommunication::handle_not_found, this, std::placeholders::_1));

        // general data
        this->ctrl_server->on("/ajax.js", std::bind(&WebCommunication::handle_js_ajax, this, std::placeholders::_1));
        this->ctrl_server->on("/general.css", std::bind(&WebCommunication::handle_css_general, this, std::placeholders::_1));
        // change network page
        this->ctrl_server->on("/network.css", std::bind(&WebCommunication::handle_network_css, this, std::placeholders::_1));
        this->ctrl_server->on("/network.js", std::bind(&WebCommunication::handle_network_js, this, std::placeholders::_1));
        this->ctrl_server->on("/network", std::bind(&WebCommunication::handle_network_html, this, std::placeholders::_1));
        // setter/getter
        this->ctrl_server->on("/set/network", std::bind(&WebCommunication::handle_network_set, this, std::placeholders::_1));
        this->ctrl_server->on("/get/network", std::bind(&WebCommunication::handle_network_get, this, std::placeholders::_1));
        this->ctrl_server->on("/get/net_list", std::bind(&WebCommunication::handle_network_get_list, this, std::placeholders::_1));
        this->ctrl_server->on("/make/net_list", std::bind(&WebCommunication::handle_network_make_list, this, std::placeholders::_1));
    }
    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
bool WebCommunication::begin()
{
    return this->_manager.begin();
}

bool WebCommunication::reconnect_ap()
{
    bool result = true;
    if (true == this->_manager.is_enable_ap()) {
        result = this->_manager.reconnect_ap(this->_flag_save);
        if (true == result) {
            log_i("MODE[A P] SSID[%s] IP[%s] ", this->get_ssid_ap(), this->get_ip_address_ap().toString());
        }
    }
    return result;
}
bool WebCommunication::reconnect_sta()
{
    bool result = true;
    if (true == this->_manager.is_enable_sta()) {
        result = this->_manager.reconnect_sta(this->_flag_save);
        if (true == result) {
            log_i("MODE[STA] SSID[%s] IP[%s] ", this->get_ssid_sta(), this->get_ip_address_sta().toString());
        }
    }
    return result;
}
#if 0
bool WebCommunication::load_default(bool save)
{
    this->_flag_save = save;
    return this->_manager.reconnect_default(save);
}
#endif
bool WebCommunication::is_connected(bool immediate)
{
    return this->_manager.is_connected(immediate);
}
void WebCommunication::load_sta_settings(bool clear)
{
    this->_manager.load_sta_settings(clear);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
AsyncWebServer *WebCommunication::get_server()
{
    return ctrl_server;
}
bool WebCommunication::is_enable_ap()
{
    return this->_manager.is_enable_ap();
}
IPAddress WebCommunication::get_ip_address_ap()
{
    return this->_manager.get_ip_address_ap();
}
const char *WebCommunication::get_ssid_ap()
{
    return this->_manager.get_ssid_ap();
}
bool WebCommunication::is_enable_sta()
{
    return this->_manager.is_enable_sta();
}
IPAddress WebCommunication::get_ip_address_sta()
{
    return this->_manager.get_ip_address_sta();
}

const char *WebCommunication::get_ssid_sta()
{
    return this->_manager.get_ssid_sta();
}
int WebCommunication::to_int(String data)
{
    if (true != data.isEmpty()) {
        int value = std::stoi(data.c_str());
        return value;
    } else {
        return 0;
    }
}
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

//////////////////////////////////////////////////////////////////////////////////////////////////////
WebCommunication::WebCommunication()
{
    this->ctrl_server = new AsyncWebServer(SETTING_WIFI_PORT);
#if CUSHY_WEB_SERVER_OTA
    AsyncElegantOTA.begin(this->ctrl_server); // Start ElegantOTA
#endif
}
WebCommunication::~WebCommunication()
{
    if (nullptr != this->ctrl_server) {
        this->ctrl_server->end();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Web
} // namespace MaSiRoProject
