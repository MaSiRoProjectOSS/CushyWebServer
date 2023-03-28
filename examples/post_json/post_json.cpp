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
    static String url = "http://localhost/post";
    static int count  = 0;

    (void)M5.update();

    if (true == M5.Btn.wasPressed()) {
        count++;
        if (100 <= count) {
            count = 0;
        }
        String reply;
        /////////////////////////
        // Json data
        /////////////////////////
        char json[255];
        sprintf(json, "{ \"j_id\": %d }", count++);
        /////////////////////////
        // Post data
        /////////////////////////
        char post[255];
        sprintf(post, "%s?id=%d", url.c_str(), count);

        if (true == cushy.post_json(post, json, &reply)) {
            log_i("%s", reply.c_str());
        } else {
            log_e("ERROR");
        }
    }
    delay(100);
}
