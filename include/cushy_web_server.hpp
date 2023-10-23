/**
 * @file CushyWebServer.hpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.0.1
 * @date 2023-03-22
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#ifndef CUSHY_WEB_SERVER_HPP
#define CUSHY_WEB_SERVER_HPP

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <Update.h>
#include <Wire.h>
#include <esp_sntp.h>
#include <functional>
#include <string.h>
#include <sys/time.h>

#ifndef CALLBACK_STYLE_USING
#define CALLBACK_STYLE_USING 0
#endif

class CushyWebServer {
public:
    enum WEB_VIEWER_MODE
    {
        NOT_INITIALIZED,
        INITIALIZED,
        DISCONNECTED,
        RETRY,
        CONNECTED_STA,
        CONNECTED_AP,
        CONNECTED_AP_AND_STA,
    };

#if CALLBACK_STYLE_USING
    using ModeFunction          = void (*)(WEB_VIEWER_MODE);
    using HandleClientFunction  = void (*)();
    using SyncCompletedFunction = void (*)();
#else
    typedef std::function<void(WEB_VIEWER_MODE)> ModeFunction;
    typedef std::function<void(void)> HandleClientFunction;
    typedef std::function<void(void)> SyncCompletedFunction;
#endif

public:
    CushyWebServer();
    ~CushyWebServer();

public:
    bool begin();
    bool post_json(String url, String payload, String *reply);
    bool post_json(String url, String payload_json, JsonDocument *reply);

public:
    void set_callback_mode(ModeFunction callback);
    void set_callback_handle_client(HandleClientFunction callback);
    void set_callback_sync_completed(SyncCompletedFunction callback);
    WEB_VIEWER_MODE get_mode();

    /////////////////////////////////////////
    // get  member valuable
    /////////////////////////////////////////
public:
    time_t millis_to_time(unsigned long ms);
    bool is_sntp_sync();
    UBaseType_t get_stack_size();
    UBaseType_t get_stack_high_water_mark();

    bool is_enable_ap();
    IPAddress get_ip_address_ap();
    const char *get_ssid_ap();

    bool is_enable_sta();
    IPAddress get_ip_address_sta();
    const char *get_ssid_sta();
    void list_reconnect_sta();

protected:
    virtual bool setup_server(AsyncWebServer *server);
    virtual void handle_favicon_ico(AsyncWebServerRequest *request);

protected:
    AsyncWebServer *get_server();

    String ip_to_string(IPAddress ip);
    byte to_byte(String data);
    int to_int(String data);
    unsigned long to_ulong(String data);
    String file_readString(const char *path);
    std::string template_json_result(bool result, std::string data = "", std::string message = "");

private:
    TaskHandle_t _task_handle;
};

#endif
