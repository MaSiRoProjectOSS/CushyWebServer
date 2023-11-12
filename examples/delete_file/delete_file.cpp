/**
 * @file delete_file.cpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.0.1
 * @date 2023-03-22
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#include "custom_cushy_web_server.hpp"

#include <Arduino.h>
#include <M5Atom.h>
#include <SPIFFS.h>
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

void readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\r\n", path);
    if (true == SPIFFS.begin()) {
        File file = fs.open(path);
        if (!file || file.isDirectory()) {
            Serial.println("- failed to open file for reading");
            return;
        }

        Serial.println("- read from file:");
        while (file.available()) {
            Serial.write(file.read());
        }
        file.close();
        SPIFFS.end();
    } else {
        Serial.println("SPIFFS Mount Failed");
    }
}

void deleteFile(fs::FS &fs, const char *path)
{
    Serial.printf("Deleting file: %s\r\n", path);
    if (true == SPIFFS.begin()) {
        if (true == fs.exists(path)) {
            if (fs.remove(path)) {
                Serial.println("- file deleted");
            } else {
                Serial.println("- delete failed");
            }
        } else {
            Serial.println("- not found");
        }
        SPIFFS.end();
    } else {
        Serial.println("SPIFFS Mount Failed");
    }
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\r\n", dirname);

    if (true == SPIFFS.begin()) {
        File root = fs.open(dirname);
        if (!root) {
            Serial.println("- failed to open directory");
            return;
        }
        if (!root.isDirectory()) {
            Serial.println(" - not a directory");
            return;
        }

        File file = root.openNextFile();
        while (file) {
            if (file.isDirectory()) {
                Serial.print("  DIR : ");
                Serial.println(file.name());
                if (levels) {
                    listDir(fs, file.path(), levels - 1);
                }
            } else {
                Serial.print("  FILE: ");
                Serial.print(file.path());
                Serial.print("\tSIZE: ");
                Serial.println(file.size());
            }
            file = root.openNextFile();
        }
        SPIFFS.end();
    } else {
        Serial.println("SPIFFS Mount Failed");
    }
}

//////////////////////////////////////////////////////////////////////////////////

void setup()
{
    (void)M5.begin(true, false, true);
    (void)M5.dis.begin();
    (void)M5.dis.fillpix(CRGB::White);
    bool result = false;
    do {
        result = cushy.begin();
        if (false == result) {
            delay(1000);
        }
    } while (false == result);
    cushy.set_callback_mode(&notify_mode);
}

void loop()
{
    static int SETTING_LOOP_TIME_SLEEP_DETECT = 100;
    (void)M5.update();
    if (true == M5.Btn.wasPressed()) {
        Serial.println("====================================");
        deleteFile(SPIFFS, "/config/wifi.ini");
        Serial.println("------------------------------------");
        listDir(SPIFFS, "/", 1);
        Serial.println("====================================");
        UBaseType_t stack_cushy_server = cushy.get_stack_high_water_mark_server();
        UBaseType_t max_cushy_server   = cushy.get_stack_size_server();
        UBaseType_t stack_cushy_wifi   = cushy.get_stack_high_water_mark_wifi();
        UBaseType_t max_cushy_wifi     = cushy.get_stack_size_wifi();

        char msg_buffer[512];
        sprintf(msg_buffer,
                "STACK SIZE : Server[%d/%d] WiFi[%d/%d]", //
                (int)stack_cushy_server,
                (int)max_cushy_server,
                (int)stack_cushy_wifi,
                (int)max_cushy_wifi);
        log_i("%s", msg_buffer);
    }
    (void)delay(SETTING_LOOP_TIME_SLEEP_DETECT);
}
