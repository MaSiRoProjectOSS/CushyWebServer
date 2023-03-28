/**
 * @file main.cpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-22
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
//#include "lib_dir.hpp"

#include <Arduino.h>
#include <M5Atom.h>
#include <SPIFFS.h>
#include <cushy_web_server.hpp>

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

class CustomCushyWebServer : public CushyWebServer {
public:
    void handle_root_html(AsyncWebServerRequest *request)
    {
        std::string html                 = "Hello !";
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain; charset=utf-8", html.c_str());
        response->addHeader("Location", String("http://") + this->get_ip().toString());
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("X-Content-Type-Options", "nosniff");
        request->send(response);
    }
    bool setup_server(AsyncWebServer *server)
    {
        server->on("/", std::bind(&CustomCushyWebServer::handle_root_html, this, std::placeholders::_1));
        return true;
    }
};

CustomCushyWebServer cushy;

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
    (void)M5.dis.fillpix(CRGB::Green);
}

void loop()
{
    static int SETTING_LOOP_TIME_SLEEP_DETECT = 1;
    (void)M5.update();
    if (true == M5.Btn.isPressed()) {
        (void)M5.dis.fillpix(CRGB::Yellow);
        Serial.println("====================================");
#if 0
        deleteFile(SPIFFS, "/config/wifi.ini");
        deleteFile(SPIFFS, "/config/wifi_00.ini");
        deleteFile(SPIFFS, "/config/wifi_01.ini");
        deleteFile(SPIFFS, "/config/wifi_02.ini");
        deleteFile(SPIFFS, "/config/wifi_03.ini");
        deleteFile(SPIFFS, "/config/wifi_04.ini");
        deleteFile(SPIFFS, "/config/wifi_05.ini");
        deleteFile(SPIFFS, "/config/wifi_06.ini");
        deleteFile(SPIFFS, "/config/wifi_07.ini");
        deleteFile(SPIFFS, "/config/wifi_08.ini");
        deleteFile(SPIFFS, "/config/wifi_09.ini");
        Serial.println("------------------------------------");
#else
        deleteFile(SPIFFS, "/config/wifi.ini");
        Serial.println("------------------------------------");
#endif
        listDir(SPIFFS, "/", 1);
        Serial.println("====================================");
        (void)M5.dis.fillpix(CRGB::Green);
    }
    (void)delay(SETTING_LOOP_TIME_SLEEP_DETECT);
}
