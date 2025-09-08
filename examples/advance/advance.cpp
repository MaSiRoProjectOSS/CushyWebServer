/**
 * @file advance.cpp
 * @brief オリジナルファビコンを設定するサンプル
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
volatile bool flag_post = false;

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

/////////////////////////////////////////////////////////////

/**
 * @brief This is a callback from the WiFi connection management thread.
 * Since it is a separate thread, it does not affect the main loop processing.
 */
void handle_client()
{
    static String url = "http://localhost/post";
    static int count  = 0;

    if (true == flag_post) {
        count++;
        if (100 <= count) {
            count = 0;
        }
        flag_post = false;
        //-------------------------
        // Json data
        //-------------------------
        char json[255];
        sprintf(json, "{ \"j_id\": %d }", count);
        //-------------------------
        // Post data
        //-------------------------
        char post[255];
        sprintf(post, "%s?id=%d", url.c_str(), count);
        log_i("SEND[%s]", post);
        ///////////////////////////////////////////////////////
        String reply;
        log_i(" * %s", "post_json(String)");
        if (true == cushy.post_json(post, json, &reply)) {
            log_i("%s", reply.c_str());
        } else {
            log_e("ERROR");
        }
        ///////////////////////////////////////////////////////
        StaticJsonDocument<255> replay2;
        log_i(" * %s", "post_json(JsonDocument)");
        if (true == cushy.post_json(post, json, &replay2)) {
            const char *replay_result = replay2["result"];
            log_i("result : %s", replay_result);
            const char *replay_status_messages = replay2["status"]["messages"];
            log_i("status.messages : %s", replay_status_messages);
            int replay_status_value = replay2["data"]["value"];
            log_i("data.value : %d", replay_status_value);
        } else {
            log_e("ERROR");
        }
    }
}

/////////////////////////////////////////////////////////////

void setup()
{
    (void)M5.begin(true, false, true);
    (void)M5.dis.begin();
    (void)M5.dis.fillpix(CRGB::White);
    delay(3000);
    log_i("------------------------------");
    log_i("CushyWebServer advance example");
    log_i("  - Custom favicon and POST sample");
    log_i("------------------------------");
    bool result = false;
    cushy.set_callback_mode(&notify_mode);
    cushy.set_callback_handle_client(&handle_client);
    delay(3000);
    do {
        result = cushy.begin();
        if (false == result) {
            delay(1000);
        }
    } while (false == result);
}

void loop()
{
    (void)M5.update();
    if (true == M5.Btn.wasReleased()) {
        flag_post = true; // send post

        Serial.println("====================================");
        UBaseType_t stack_cushy_server = cushy.get_stack_high_water_mark_server();
        UBaseType_t max_cushy_server   = cushy.get_stack_size_server();
        UBaseType_t stack_cushy_wifi   = cushy.get_stack_high_water_mark_wifi();
        UBaseType_t max_cushy_wifi     = cushy.get_stack_size_wifi();

        Serial.printf("STACK SIZE : Server[%d/%d] WiFi[%d/%d]\n", //
                      (int)(max_cushy_server - stack_cushy_server),
                      (int)max_cushy_server,
                      (int)(max_cushy_wifi - stack_cushy_wifi),
                      (int)max_cushy_wifi);
        Serial.println("====================================");
    }
    delay(100);
}
