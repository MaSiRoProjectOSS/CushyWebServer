/**
 * @file file_operations.hpp
 * @brief
 * @version 0.5.0
 * @date 2025-09-08
 *
 * @copyright Copyright (c) 2025 / MaSiRo Project.
 *
 */
#include <Arduino.h>
#include <SPIFFS.h>

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

void readFile(fs::FS &fs, const char *path)
{
    if (true == SPIFFS.begin()) {
        File file     = fs.open(path);
        int bytesRead = file.size();
        Serial.printf("Reading file: %s[%d]\r\n", path, file.size());

        if (!file || file.isDirectory()) {
            Serial.println("- failed to open file for reading");
            return;
        }

        while (file.available()) {
            Serial.write(file.read());
            bytesRead--;
            if (0 > bytesRead) {
                Serial.println("** Too many data. **");
                break;
            }
        }
        file.close();
        SPIFFS.end();
        Serial.println("<EOF>");
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
