/**
 * @file standard.cpp
 * @brief SPIFFSファイル操作とWi-Fi/AP情報を表示・確認するサンプル
 * @version 0.0.1
 * @date 2023-03-22
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#include "custom_cushy_web_server.hpp"
#include "file_operations.hpp"
#include "src/setting_cushy_web_server.hpp"

#include <Arduino.h>
#include <M5Atom.h>
#include <cushy_web_server.hpp>

CustomCushyWebServer cushy;

//////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////

void setup()
{
    (void)M5.begin(true, false, true);
    (void)M5.dis.begin();
    (void)M5.dis.fillpix(CRGB::White);
    delay(3000);
    log_i("------------------------------");
    log_i("CushyWebServer standard example");
    log_i("  - SPIFFS file operations and Wi-Fi/AP information display sample");
    log_i("------------------------------");
    bool result = false;
    cushy.set_callback_mode(&notify_mode);
    do {
        result = cushy.begin();
        if (false == result) {
            delay(1000);
        }
    } while (false == result);
}

void loop()
{
    static int SETTING_LOOP_TIME_SLEEP_DETECT = 100;
    static int previous_btn_time_ms           = 0;
    (void)M5.update();
    int options = 0;

    int current_time_ms = millis();
    // クリック時間で動作を変更する
    // 300ms未満：設定ファイル削除
    // 300ms以上1秒未満：STA再接続
    // 1秒以上：設定ファイル表示
    if (true == M5.Btn.wasReleased()) {
        if (0 == previous_btn_time_ms) {
            previous_btn_time_ms = current_time_ms;
        } else if (300 > (current_time_ms - previous_btn_time_ms)) {
            options              = 1;
            previous_btn_time_ms = 0;
        } else if (1000 > (current_time_ms - previous_btn_time_ms)) {
            options              = 2;
            previous_btn_time_ms = 0;
        }
    }
    if (0 != previous_btn_time_ms) {
        if (1000 <= (current_time_ms - previous_btn_time_ms)) {
            options              = 3;
            previous_btn_time_ms = 0;
        }
    }

    switch (options) {
        case 0:
            /* code */
            break;
        case 1:
            Serial.println("====================================");
            Serial.println("Delete all setting files");
            Serial.println("====================================");
            deleteFile(SPIFFS, SETTING_WIFI_SETTING_FILE);
            deleteFile(SPIFFS, SETTING_WIFI_AP_SETTING_FILE);
            deleteFile(SPIFFS, SETTING_WIFI_STA_CONNECTED_FILE);
            for (int i = 0; i < SETTING_WIFI_STA_FILE_MAX; i++) {
                char buffer[32];
                sprintf(buffer, SETTING_WIFI_STA_FILE_PATTERN, i);
                deleteFile(SPIFFS, buffer);
            }
            Serial.println("------------------------------------");
            listDir(SPIFFS, "/", 1);
            break;
        case 2:
            Serial.println("====================================");
            Serial.println("reconnect sta/ap");
            Serial.println("====================================");
            cushy.reconnect(CushyWebServer::NETWORK_INTERFACE::NW_IF_WIFI_STA);
            cushy.reconnect(CushyWebServer::NETWORK_INTERFACE::NW_IF_WIFI_AP);
            break;
        case 3:
            Serial.println("====================================");
            Serial.println("Show all setting files");
            Serial.println("====================================");
            listDir(SPIFFS, "/", 1);
            Serial.println("------------------------------------");
            readFile(SPIFFS, SETTING_WIFI_SETTING_FILE);
            readFile(SPIFFS, SETTING_WIFI_AP_SETTING_FILE);
            readFile(SPIFFS, SETTING_WIFI_STA_CONNECTED_FILE);
            for (int i = 0; i < SETTING_WIFI_STA_FILE_MAX; i++) {
                char buffer[32];
                sprintf(buffer, SETTING_WIFI_STA_FILE_PATTERN, i);
                readFile(SPIFFS, buffer);
            }
            Serial.println("====================================");
            Serial.println("Show Wi-Fi/AP information");
            Serial.println("====================================");
            {
                UBaseType_t stack_cushy_server = cushy.get_stack_high_water_mark_server();
                UBaseType_t max_cushy_server   = cushy.get_stack_size_server();
                UBaseType_t stack_cushy_wifi   = cushy.get_stack_high_water_mark_wifi();
                UBaseType_t max_cushy_wifi     = cushy.get_stack_size_wifi();

                Serial.printf("STACK SIZE : Server[%d/%d] WiFi[%d/%d]\n", //
                              (int)(max_cushy_server - stack_cushy_server),
                              (int)max_cushy_server,
                              (int)(max_cushy_wifi - stack_cushy_wifi),
                              (int)max_cushy_wifi);
            }
            {
                bool enable     = false;
                String ssid     = "";
                String hostname = "";
                String ip       = "";
                cushy.get_information(CushyWebServer::NETWORK_INTERFACE::NW_IF_WIFI_AP, enable, ssid, hostname, ip);
                Serial.printf("AP : [%s:%s] SSID[%s] HOSTNAME[%s] IP[%s]\n", //
                              (true == enable) ? "Enabled" : "Disabled",
                              cushy.is_connected(CushyWebServer::NETWORK_INTERFACE::NW_IF_WIFI_AP) ? "Connected" : "Disconnected",
                              ssid.c_str(),
                              hostname.c_str(),
                              ip.c_str());
                cushy.get_information(CushyWebServer::NETWORK_INTERFACE::NW_IF_WIFI_STA, enable, ssid, hostname, ip);
                Serial.printf("STA: [%s:%s] SSID[%s] HOSTNAME[%s] IP[%s]\n", //
                              (true == enable) ? "Enabled" : "Disabled",
                              cushy.is_connected(CushyWebServer::NETWORK_INTERFACE::NW_IF_WIFI_STA) ? "Connected" : "Disconnected",
                              ssid.c_str(),
                              hostname.c_str(),
                              ip.c_str());
            }
            break;
        default:
            break;
    }

    (void)delay(SETTING_LOOP_TIME_SLEEP_DETECT);
}
