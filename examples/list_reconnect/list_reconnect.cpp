/**
 * @file list_reconnect.cpp
 * @brief
 * @version 0.4.2
 * @date 2023-10-30
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#include "custom_cushy_web_server.hpp"

#include <Arduino.h>
#include <M5Atom.h>

CustomCushyWebServer cushy;

void notify_mode(CushyWebServer::WEB_VIEWER_MODE mode)
{
    switch (mode) {
        case CushyWebServer::WEB_VIEWER_MODE::NOT_INITIALIZED:
            log_i("NOT_INITIALIZED");
            (void)M5.dis.fillpix(CRGB::Aqua);
            break;
        case CushyWebServer::WEB_VIEWER_MODE::INITIALIZED:
            log_i("INITIALIZED");
            (void)M5.dis.fillpix(CRGB::Yellow);
            break;
        case CushyWebServer::WEB_VIEWER_MODE::DISCONNECTED:
            log_i("DISCONNECTED");
            (void)M5.dis.fillpix(CRGB::DarkRed);
            break;
        case CushyWebServer::WEB_VIEWER_MODE::RETRY:
            log_i("RETRY");
            (void)M5.dis.fillpix(CRGB::Red);
            break;
        case CushyWebServer::WEB_VIEWER_MODE::CONNECTED_STA:
            log_i("CONNECTED_STA");
            (void)M5.dis.fillpix(CRGB::Green);
            break;
        case CushyWebServer::WEB_VIEWER_MODE::CONNECTED_AP:
            log_i("CONNECTED_AP");
            (void)M5.dis.fillpix(CRGB::Blue);
            break;
        case CushyWebServer::WEB_VIEWER_MODE::CONNECTED_AP_AND_STA:
            log_i("CONNECTED_AP_AND_STA");
            (void)M5.dis.fillpix(CRGB::Blue);
            break;
        default:
            (void)M5.dis.fillpix(CRGB::Black);
            break;
    }
}

void setup()
{
    (void)M5.begin(true, false, true);
    (void)M5.dis.begin();
    (void)M5.dis.fillpix(CRGB::White);
    bool result = false;
    cushy.set_callback_mode(&notify_mode);
    do {
        result = cushy.begin();
        if (false == result) {
            delay(1000);
        }
    } while (false == result);
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    File root = fs.open(dirname);
    if (root) {
        File file = root.openNextFile();
        while (file) {
            if (file.isDirectory()) {
                if (levels) {
                    listDir(fs, file.name(), levels - 1);
                }
            } else {
                log_i("\t[SIZE: %7d] %s", file.size(), file.path());
            }
            file = root.openNextFile();
        }
    }
}

void listDir(const char *dirname, uint8_t levels)
{
    try {
        if (SPIFFS.begin(true)) {
            listDir(SPIFFS, dirname, levels);
            SPIFFS.end();
        }
    } catch (...) {
    }
}

void loop()
{
    M5.update();
    if (true == M5.Btn.wasPressed()) {
        // listDir
        listDir("/config", 2);
        // reconnect_sta
        log_i("reconnect_sta");
        cushy.reconnect_sta();

        // STACK SIZE
        UBaseType_t stack_cushy_server = cushy.get_stack_high_water_mark_server();
        UBaseType_t max_cushy_server   = cushy.get_stack_size_server();
        UBaseType_t stack_cushy_wifi   = cushy.get_stack_high_water_mark_wifi();
        UBaseType_t max_cushy_wifi     = cushy.get_stack_size_wifi();

        char msg_buffer[512];
        sprintf(msg_buffer,
                "STACK SIZE : Server[%d/%d] WiFi[%d/%d]", //
                (int)(max_cushy_server - stack_cushy_server),
                (int)max_cushy_server,
                (int)(max_cushy_wifi - stack_cushy_wifi),
                (int)max_cushy_wifi);
        log_i("%s", msg_buffer);
    }
    delay(25);
}
