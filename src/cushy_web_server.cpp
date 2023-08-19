/**
 * @file CushyWebServer.cpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.0.1
 * @date 2023-03-22
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#include "cushy_web_server.hpp"

#include "impl/web_communication.hpp"
#include "setting_cushy_web_server.hpp"

MaSiRoProject::Web::WebCommunication ctrl_web;

#pragma region thread_fuction
///////////////////////////////////////////////////////////////////////////////////////////////

const char *THREAD_NAME_SERVER       = "ThreadServer";
const char *THREAD_NAME_WIFI         = "ThreadWiFi";
const int THREAD_SEEK_INTERVAL_WIFI  = (10 * 1000);
const int THREAD_RETRY_INTERVAL_WIFI = (5 * 1000);
const int THREAD_INTERVAL_WIFI       = (25);

volatile bool flag_list_reconnect            = false;
volatile bool flag_thread_wifi_initialized   = false;
volatile bool flag_thread_wifi_fin           = false;
volatile bool flag_thread_server_initialized = false;
volatile bool flag_thread_server_fin         = false;
CushyWebServer::WEB_VIEWER_MODE _mode        = CushyWebServer::WEB_VIEWER_MODE::NOT_INITIALIZED;
CushyWebServer::ModeFunction _callback_mode;
CushyWebServer::HandleClientFunction _callback_handle_client;
CushyWebServer::SyncCompletedFunction _callback_sync_completed;
volatile time_t t0           = 0;
volatile unsigned long t1    = 0;
volatile bool flag_sntp_sync = false;

void sntp_time_sync_notification(struct timeval *tv)
{
    if (SNTP_SYNC_STATUS_COMPLETED == sntp_get_sync_status()) {
        t1 = millis();
        t0 = time(NULL);
        log_v("%s", "SNTP Sync completed");
        flag_sntp_sync = true;
        if (nullptr != _callback_sync_completed) {
            _callback_sync_completed();
        }
    }
}

void call_handle_client()
{
    if (nullptr != _callback_handle_client) {
        _callback_handle_client();
    }
}

void set_mode(CushyWebServer::WEB_VIEWER_MODE mode)
{
    if (_mode != mode) {
        _mode = mode;
        if (nullptr != _callback_mode) {
            _callback_mode(mode);
        }
    }
}

void thread_server(void *args)
{
    flag_thread_server_initialized                = true;
    flag_thread_server_fin                        = false;
    bool flag_wait                                = true;
    bool flag_once                                = false;
    CushyWebServer::WEB_VIEWER_MODE previous_mode = CushyWebServer::WEB_VIEWER_MODE::NOT_INITIALIZED;

    while (false == flag_thread_server_fin) {
        try {
            while (true == flag_wait) {
                switch (_mode) {
                    case CushyWebServer::WEB_VIEWER_MODE::CONNECTED_AP_AND_STA:
                    case CushyWebServer::WEB_VIEWER_MODE::CONNECTED_STA:
                    case CushyWebServer::WEB_VIEWER_MODE::CONNECTED_AP:
                        flag_wait = false;
                        break;

                    default:
                        break;
                }
                vTaskDelay(THREAD_INTERVAL_WIFI);
            }
            previous_mode = _mode;
            if (false == flag_once) {
                flag_once = true;
#if SETTING_SNTP_ENABLE
                if (false == flag_sntp_sync) {
                    flag_sntp_sync = true;
                    configTzTime(SETTING_SNTP_TIME_ZONE, SETTING_SNTP_SERVER);
                    sntp_set_time_sync_notification_cb(sntp_time_sync_notification);
                }
#endif
            }
            if (nullptr != ctrl_web.get_server()) {
                log_d("%s", "Start server");
                ctrl_web.get_server()->begin();
                while (false == flag_thread_server_fin) {
                    call_handle_client();
                    if (previous_mode != _mode) {
                        break;
                    }
                    vTaskDelay(THREAD_INTERVAL_WIFI);
                }
                ctrl_web.get_server()->end();
                log_d("%s", "End server");
            }
            vTaskDelay(THREAD_RETRY_INTERVAL_WIFI);
        } catch (...) {
        }
    }
    flag_thread_server_initialized = false;
}
void thread_wifi(void *args)
{
    unsigned long err_begin = millis() + (1000 * SETTING_WIFI_STA_AUTO_TRANSITIONS_TIMEOUT);
#if SETTING_SNTP_ENABLE
    bool flag_sntp_sync = false;
#endif
    CushyWebServer::WEB_VIEWER_MODE previous_mode = CushyWebServer::WEB_VIEWER_MODE::NOT_INITIALIZED;

    flag_thread_wifi_initialized = true;
    flag_thread_wifi_fin         = false;
    set_mode(CushyWebServer::WEB_VIEWER_MODE::NOT_INITIALIZED);

    while (false == ctrl_web.begin()) {
        if (true == flag_thread_wifi_fin) {
            break;
        }
        vTaskDelay(THREAD_SEEK_INTERVAL_WIFI);
    }
    set_mode(CushyWebServer::WEB_VIEWER_MODE::INITIALIZED);
    if (true == ctrl_web.is_enable_ap()) {
        while (false == ctrl_web.reconnect_ap()) {
            if (true == flag_thread_wifi_fin) {
                break;
            }
            vTaskDelay(THREAD_SEEK_INTERVAL_WIFI);
        }
        set_mode(CushyWebServer::WEB_VIEWER_MODE::CONNECTED_AP);
    }

    while (false == flag_thread_wifi_fin) {
        try {
            bool connected_ap  = ctrl_web.is_enable_ap();
            bool connected_sta = ctrl_web.is_enable_sta();
            if ((true == connected_ap) && (true == connected_sta)) {
                if (previous_mode != _mode) {
                    log_d("%s", "connect mode : AP and STA");
                }
                set_mode(CushyWebServer::WEB_VIEWER_MODE::CONNECTED_AP_AND_STA);
            } else if (true == connected_ap) {
                if (previous_mode != _mode) {
                    log_d("%s", "connect mode : AP");
                }
                set_mode(CushyWebServer::WEB_VIEWER_MODE::CONNECTED_AP);
            } else if (true == connected_sta) {
                if (previous_mode != _mode) {
                    log_d("%s", "connect mode : STA");
                }
                set_mode(CushyWebServer::WEB_VIEWER_MODE::CONNECTED_STA);
            } else {
                if (previous_mode != _mode) {
                    log_d("%s", "connect mode : DISCONNECTED");
                }
                set_mode(CushyWebServer::WEB_VIEWER_MODE::DISCONNECTED);
            }
            previous_mode = _mode;
            if (false == ctrl_web.is_enable_sta()) {
                // do nothing
            } else {
                if (false == ctrl_web.reconnect_sta()) {
                    set_mode(CushyWebServer::WEB_VIEWER_MODE::RETRY);
                    if (err_begin < millis()) {
                        err_begin = millis() + (1000 * SETTING_WIFI_STA_AUTO_TRANSITIONS_TIMEOUT);
                        ctrl_web.load_sta_settings(false);
                        log_d("%s", "Retry to connect");
                    }
                } else {
                    while (false == flag_thread_wifi_fin) {
                        if (true != ctrl_web.is_connected()) {
                            break;
                        }
                        if (true == flag_list_reconnect) {
                            flag_list_reconnect = false;
                            ctrl_web.load_sta_settings(true);
                            break;
                        }
                        vTaskDelay(THREAD_RETRY_INTERVAL_WIFI);
                    }
                    set_mode(CushyWebServer::WEB_VIEWER_MODE::DISCONNECTED);
                }
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
    flag_thread_server_fin = true;
    flag_thread_wifi_fin   = true;
}

bool CushyWebServer::begin()
{
    bool result = false;
    try {
        ////////////////////////////////////////////////////////
        if (true == ctrl_web.setup()) {
            ctrl_web.get_server()->on("/favicon.ico", std::bind(&CushyWebServer::handle_favicon_ico, this, std::placeholders::_1));
            if (true == this->setup_server(ctrl_web.get_server())) {
                if (false == flag_thread_wifi_initialized) {
                    xTaskCreatePinnedToCore(thread_wifi, //
                                            THREAD_NAME_WIFI,
                                            SETTING_THREAD_TASK_ASSIGNED_SIZE_WIFI,
                                            NULL,
                                            SETTING_THREAD_PRIORITY_WIFI,
                                            &this->_task_handle,
                                            SETTING_THREAD_CORE_WIFI);
                    xTaskCreatePinnedToCore(thread_server, //
                                            THREAD_NAME_SERVER,
                                            SETTING_THREAD_TASK_ASSIGNED_SIZE_SERVER,
                                            NULL,
                                            SETTING_THREAD_PRIORITY_SERVER,
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
void CushyWebServer::list_reconnect_sta()
{
    flag_list_reconnect = true;
}

bool CushyWebServer::is_enable_ap()
{
    return ctrl_web.is_enable_ap();
}
IPAddress CushyWebServer::get_ip_address_ap()
{
    return ctrl_web.get_ip_address_ap();
}
const char *CushyWebServer::get_ssid_ap()
{
    return ctrl_web.get_ssid_ap();
}

bool CushyWebServer::is_enable_sta()
{
    return ctrl_web.is_enable_sta();
}
IPAddress CushyWebServer::get_ip_address_sta()
{
    return ctrl_web.get_ip_address_sta();
}
const char *CushyWebServer::get_ssid_sta()
{
    return ctrl_web.get_ssid_sta();
}

void CushyWebServer::handle_favicon_ico(AsyncWebServerRequest *request)
{
    ctrl_web.handle_favicon_ico(request);
}

void CushyWebServer::set_callback_mode(ModeFunction callback)
{
    _callback_mode = callback;
}
void CushyWebServer::set_callback_handle_client(HandleClientFunction callback)
{
    _callback_handle_client = callback;
}
void CushyWebServer::set_callback_sync_completed(SyncCompletedFunction callback)
{
    _callback_sync_completed = callback;
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
// get member valuable
/////////////////////////////////////////
time_t CushyWebServer::millis_to_time(unsigned long ms)
{
    return t0 + (long)(((long long)ms - (long long)t1) / 1000);
}
bool CushyWebServer::is_sntp_sync()
{
    return flag_sntp_sync;
}
UBaseType_t CushyWebServer::get_stack_size()
{
    return SETTING_THREAD_TASK_ASSIGNED_SIZE_SERVER;
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
    if ("" != url.c_str()) {
        if (true == http.begin(url.c_str())) {
            http.addHeader("Content-Type", "application/json");
            int httpCode = http.POST(payload_json);

            if (httpCode > 0) {
                if (HTTP_CODE_OK == httpCode) {
                    log_d("[HTTP] POST... code: %d", httpCode);
                    *reply = http.getString();
                    result = true;
                } else {
                    log_w("[HTTP] POST... code: %d", httpCode);
                }
            } else {
                log_w("[HTTP] POST... failed, error: %s", http.errorToString(httpCode).c_str());
            }
            http.end();
        } else {
            log_w("[HTTP] connection refused");
        }
    }
    return result;
}

bool CushyWebServer::post_json(String url, String payload_json, JsonDocument *reply)
{
    bool result = false;
    HTTPClient http;
    if ("" != url.c_str()) {
        if (true == http.begin(url.c_str())) {
            http.addHeader("Content-Type", "application/json");
            int httpCode = http.POST(payload_json);

            if (httpCode > 0) {
                if (HTTP_CODE_OK == httpCode) {
                    log_d("[HTTP] POST... code: %d", httpCode);
                    DeserializationError error = deserializeJson(*reply, http.getString());
                    switch (error.code()) {
                        case DeserializationError::Ok:
                            result = true;
                            break;
                        default:
                            log_w("[HTTP] deserialize error : %s", error.c_str());
                            break;
                    }
                } else {
                    log_w("[HTTP] POST... code: %d", httpCode);
                }
            } else {
                log_w("[HTTP] POST... failed, error: %s", http.errorToString(httpCode).c_str());
            }
            http.end();
        } else {
            log_w("[HTTP] connection refused");
        }
    }
    return result;
}
