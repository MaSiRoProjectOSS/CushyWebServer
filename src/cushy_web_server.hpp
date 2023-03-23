/**
 * @file CushyWebServer.hpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-22
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#ifndef CUSHY_WEB_SERVER_HPP
#define CUSHY_WEB_SERVER_HPP

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <functional>
#include <string.h>

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
    };

    using ModeFunction = void (*)(WEB_VIEWER_MODE);

public:
    CushyWebServer();
    ~CushyWebServer();

public:
    bool begin();
    bool post_json(String url, String payload, String *reply);

public:
    void set_callback_mode(ModeFunction callback);
    WEB_VIEWER_MODE get_mode();

    /////////////////////////////////////////
    // get  member valuable
    /////////////////////////////////////////
public:
    UBaseType_t get_stack_size();
    UBaseType_t get_stack_high_water_mark();

protected:
    virtual bool setup_server(AsyncWebServer *server);
    AsyncWebServer *get_server();
    IPAddress get_ip();

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
