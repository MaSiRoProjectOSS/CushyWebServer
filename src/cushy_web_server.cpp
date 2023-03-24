/**
 * @file CushyWebServer.cpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-22
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#include "cushy_web_server.hpp"

#include "impl/web_communication.hpp"
#include "setting_cushy_web_server.hpp"

#include <Arduino.h>

MaSiRoProject::Web::WebCommunication ctrl_web;

#pragma region thread_fuction
///////////////////////////////////////////////////////////////////////////////////////////////

const char *THREAD_NAME_WIFI         = "ThreadWiFi";
const int THREAD_SEEK_INTERVAL_WIFI  = (10 * 1000);
const int THREAD_RETRY_INTERVAL_WIFI = (5 * 1000);
const int THREAD_INTERVAL_WIFI       = (25);

volatile bool flag_thread_wifi_initialized = false;
volatile bool flag_thread_wifi_fin         = false;
CushyWebServer::WEB_VIEWER_MODE _mode      = CushyWebServer::WEB_VIEWER_MODE::NOT_INITIALIZED;
CushyWebServer::ModeFunction _callback_mode;

void set_mode(CushyWebServer::WEB_VIEWER_MODE mode)
{
    if (_mode != mode) {
        _mode = mode;
        if (nullptr != _callback_mode) {
            _callback_mode(mode);
        }
    }
}

void thread_wifi(void *args)
{
#if SETTING_WIFI_MODE_AUTO_TRANSITIONS
    unsigned long err_begin = millis() + SETTING_WIFI_AUTO_TRANSITIONS_DEFAULT_TIMEOUT;
#endif
    flag_thread_wifi_fin = false;
    set_mode(CushyWebServer::WEB_VIEWER_MODE::NOT_INITIALIZED);

    while (false == ctrl_web.begin()) {
        if (true == flag_thread_wifi_fin) {
            break;
        }
        vTaskDelay(THREAD_SEEK_INTERVAL_WIFI);
    }
    set_mode(CushyWebServer::WEB_VIEWER_MODE::INITIALIZED);

    while (false == flag_thread_wifi_fin) {
        try {
            if (false == ctrl_web.reconnect()) {
                set_mode(CushyWebServer::WEB_VIEWER_MODE::RETRY);
#if SETTING_WIFI_MODE_AUTO_TRANSITIONS
                if (true != ctrl_web.is_ap_mode()) {
                    if (err_begin < millis()) {
                        err_begin = millis() + SETTING_WIFI_AUTO_TRANSITIONS_DEFAULT_TIMEOUT;
                        ctrl_web.load_default(false);
                    }
                }
#endif
            } else {
                set_mode((true == ctrl_web.is_ap_mode()) ? //
                                 CushyWebServer::WEB_VIEWER_MODE::CONNECTED_AP :
                                 CushyWebServer::WEB_VIEWER_MODE::CONNECTED_STA);
                if (nullptr != ctrl_web.get_server()) {
                    ctrl_web.get_server()->begin();
                    while (false == flag_thread_wifi_fin) {
                        if (true != ctrl_web.is_connected()) {
                            break;
                        }
                        vTaskDelay(THREAD_INTERVAL_WIFI);
                    }
                    ctrl_web.get_server()->end();
                }
                set_mode(CushyWebServer::WEB_VIEWER_MODE::DISCONNECTED);
            }
            vTaskDelay(THREAD_RETRY_INTERVAL_WIFI);
        } catch (...) {
        }
    }
    flag_thread_wifi_initialized = false;
}
///////////////////////////////////////////////////////////////////////////////////////////////
#pragma endregion

///////////////////////////////////////////////////////////////////////////////////////////////
CushyWebServer::CushyWebServer()
{
}
CushyWebServer::~CushyWebServer()
{
    flag_thread_wifi_fin = true;
}

bool CushyWebServer::begin()
{
    bool result = false;
    try {
        ////////////////////////////////////////////////////////
        if (true == ctrl_web.setup()) {
            if (true == this->setup_server(ctrl_web.get_server())) {
                if (false == flag_thread_wifi_initialized) {
                    flag_thread_wifi_initialized = true;
                    xTaskCreatePinnedToCore(thread_wifi, //
                                            THREAD_NAME_WIFI,
                                            SETTING_TASK_ASSIGNED_SIZE,
                                            NULL,
                                            SETTING_THREAD_PRIORITY,
                                            &this->_task_handle,
                                            SETTING_THREAD_CORE_WIFI);
                }
                result = true;
            }
        }

        ////////////////////////////////////////////////////////
    } catch (...) {
    }
    return result;
}

bool CushyWebServer::setup_server(AsyncWebServer *server)
{
    return true;
}
AsyncWebServer *CushyWebServer::get_server()
{
    return ctrl_web.get_server();
}
IPAddress CushyWebServer::get_ip()
{
    return ctrl_web.get_ip();
}

void CushyWebServer::set_callback_mode(ModeFunction callback)
{
    _callback_mode = callback;
}
CushyWebServer::WEB_VIEWER_MODE CushyWebServer::get_mode()
{
    return _mode;
}
/////////////////////////////////
// value conversion
/////////////////////////////////
String CushyWebServer::ip_to_string(IPAddress ip)
{
    String res = "";
    for (int i = 0; i < 3; i++) {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}
byte CushyWebServer::to_byte(String data)
{
    if (true != data.isEmpty()) {
        int value = std::stoi(data.c_str());
        return (byte)(value);
    } else {
        return 0;
    }
}
unsigned long CushyWebServer::to_ulong(String data)
{
    if (true != data.isEmpty()) {
        unsigned long value = std::stoul(data.c_str());
        return value;
    } else {
        return 0;
    }
}
int CushyWebServer::to_int(String data)
{
    if (true != data.isEmpty()) {
        int value = std::stoi(data.c_str());
        return value;
    } else {
        return 0;
    }
}
String CushyWebServer::file_readString(const char *path)
{
    return ctrl_web.file_readString(path);
}
std::string CushyWebServer::template_json_result(bool result, std::string data, std::string message)
{
    return ctrl_web.template_json_result(result, data, message);
}

/////////////////////////////////////////
// get  member valuable
/////////////////////////////////////////
UBaseType_t CushyWebServer::get_stack_size()
{
    return SETTING_TASK_ASSIGNED_SIZE;
}
UBaseType_t CushyWebServer::get_stack_high_water_mark()
{
    return uxTaskGetStackHighWaterMark(this->_task_handle);
}

bool CushyWebServer::post_json(String url, String payload_json, String *reply)
{
    bool result = false;
    HTTPClient http;
    *reply = "";
    if (true == http.begin(url.c_str())) {
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST(payload_json.c_str());

        if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            log_d("[HTTP] POST... code: %d", httpCode);
            // file found at server
            if (HTTP_CODE_OK == httpCode) {
                *reply = http.getString();
                result = true;
            }
        } else {
            log_e("[HTTP] POST... failed, error: %s", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
    return result;
}
