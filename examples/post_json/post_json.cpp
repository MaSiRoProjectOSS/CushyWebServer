/**
 * @file mode_notification.ino
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
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
volatile bool flag = false;

/**
 * @brief This is a callback from the WiFi connection management thread.
 * Since it is a separate thread, it does not affect the main loop processing.
 */
void handle_client()
{
    static String url = "http://localhost/post";
    static int count  = 0;

    if (true == flag) {
        count++;
        if (100 <= count) {
            count = 0;
        }
        flag = false;
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

void setup()
{
    (void)M5.begin(true, false, true);
    (void)M5.dis.begin();
    (void)M5.dis.fillpix(CRGB::White);
    bool result = false;
    cushy.set_callback_handle_client(&handle_client);
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
    (void)M5.update();
    if (true == M5.Btn.wasPressed()) {
        flag = true;
    }
    delay(100);
}
