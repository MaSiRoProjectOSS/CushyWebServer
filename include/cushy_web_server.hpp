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
    typedef enum
    {
        NOT_INITIALIZED,
        INITIALIZED,
        DISCONNECTED,
        RETRY,
        CONNECTED_STA,
        CONNECTED_AP,
        CONNECTED_AP_AND_STA,
    } WEB_VIEWER_MODE;
    typedef enum
    {
        NW_IF_WIFI_STA = 0, /**< station interface */
        NW_IF_WIFI_AP,      /**< soft-AP interface */
        NW_IF_ETH,          /**< ethernet interface */
    } NETWORK_INTERFACE;

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
    //////////////////////////////////////////////////////////////
    // Constructor and destructor
    //////////////////////////////////////////////////////////////
    CushyWebServer();
    ~CushyWebServer();

public:
    //////////////////////////////////////////////////////////////
    // Begin and callback settings
    //////////////////////////////////////////////////////////////
    bool begin();
    void set_callback_mode(ModeFunction callback);
    void set_callback_handle_client(HandleClientFunction callback);
    void set_callback_sync_completed(SyncCompletedFunction callback);

protected:
    //////////////////////////////////////////////////////////////
    // Virtual functions
    //////////////////////////////////////////////////////////////
    virtual bool setup_server(AsyncWebServer *server);
    virtual void handle_favicon_ico(AsyncWebServerRequest *request);
    virtual void handle_not_found(AsyncWebServerRequest *request);

public:
    //////////////////////////////////////////////////////////////
    // Post functions
    //////////////////////////////////////////////////////////////
    bool post_json(String url, String payload, String *reply);
    bool post_json(String url, String payload_json, JsonDocument *reply);

public:
    //////////////////////////////////////////////////////////////
    // Getter and Setter
    //////////////////////////////////////////////////////////////
    void reconnect_sta();
    bool is_sntp_sync();
    bool is_enable(NETWORK_INTERFACE interface);
    IPAddress get_ip_address(NETWORK_INTERFACE interface);
    const char *get_ssid(NETWORK_INTERFACE interface);
    WEB_VIEWER_MODE get_mode();
    time_t millis_to_time(unsigned long ms);

    bool set_enable(NETWORK_INTERFACE interface, bool flag);
    bool save_ap_setting(bool enable, std::string ssid, std::string pass);
    bool save_sta_setting(bool enable, std::string ssid, std::string pass, std::string hostname, int num);

public:
    //////////////////////////////////////////////////////////////
    // Stack function
    //////////////////////////////////////////////////////////////
    UBaseType_t get_stack_size_wifi();
    UBaseType_t get_stack_high_water_mark_wifi();
    UBaseType_t get_stack_size_server();
    UBaseType_t get_stack_high_water_mark_server();

protected:
    //////////////////////////////////////////////////////////////
    // Protected functions
    //////////////////////////////////////////////////////////////
    String ip_to_string(IPAddress ip);
    byte to_byte(String data);
    int to_int(String data);
    unsigned long to_ulong(String data);
    String file_readString(const char *path);
    std::string template_json_result(bool result, std::string data = "", std::string message = "");
    AsyncWebServer *get_server();

private:
    TaskHandle_t _task_handle_wifi;
    TaskHandle_t _task_handle_server;
};

#endif
