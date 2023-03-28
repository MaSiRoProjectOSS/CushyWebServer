/**
 * @file original_favicon.ino
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * # オリジナルファビコンを設定する方法
 *  CushyWebServerを継承したクラスでhandle_favicon_ico()をoverrideし、
 *  imageデータ戻してください。
 *
 *  サンプルはpngファイルのバイナリデータを「data_custom_cushy_web_server.hpp」に設定してます。
 * @version 0.3.0
 * @date 2023-03-28
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#include "custom_cushy_web_server.hpp"

#include <Arduino.h>
#include <M5Atom.h>

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
}
