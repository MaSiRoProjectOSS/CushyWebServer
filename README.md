# Cushy Web Server

ESP32に下記の機能を持たせるためのライブラリ

* WebServer
  * OTA(HTTP)
  * WiFi切り替えページ

## Project Status

<div style="display: flex">
    <div style="width:1em; background-color: red;margin-right:1em;"></div>
    <span style="">This project will not be refurbished unless requested.</span>
</div>

## Description

CushyWebServer クラスを継承する事でESP32(M5Stack)で動作する WebServerを容易に立てるライブラリです。
ページの追加方法は、[ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) をご参照ください。

関数```cushy.begin()```を実行することで、Core1にスレッドを立ててネットワーク接続まで実施する。

[ayushsharma82/AsyncElegantOTA](https://github.com/ayushsharma82/AsyncElegantOTA)を使用しおりURL```http://(アドレス)/update```でバイナリの書き換えが行える。

ネットワーク切り替えページ```http://(アドレス)/network```を持っており、接続先をブラウザから切り替える事ができる。

このライブラリの設定は```src\setting_cushy_web_server.hpp```で管理しており、```plantformio.ini```の```build_flags```で制御することを想定している。


接続先情報はSPIFFSに持っており、(1)SPIFFSにあるファイル(2)```src\setting_cushy_web_server.hpp```の定義の順で参照します。


## Installation

This system is compiled using PlatformIO. Please install the extension from VS Code's extension recommendations. If you want to know more, check [here](https://docs.platformio.org/en/latest/).

* platformio.ini

```ini
[env:m5stack_atom]
platform = espressif32
board = m5stack-atom
framework = arduino
lib_deps =
	m5stack/M5Atom@^0.1.0
	fastled/FastLED@^3.5.0
	https://github.com/MaSiRoProjectOSS/CushyWebServer
```

* main.cpp

```c++
#include <Arduino.h>
#include <M5Atom.h>
#include <cushy_web_server.hpp>

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
}
```

## Usage

設定ファイル```src\setting_cushy_web_server.hpp```で定義したdefineは下記の通りです。

| define                                        |            デフォルト値 | 定義                                                                                                               |
| :-------------------------------------------- | ----------------------: | :----------------------------------------------------------------------------------------------------------------- |
| SETTING_THREAD_CORE_WIFI                      |                     (1) | スレッドを動作させているCore番号                                                                                   |
| SETTING_THREAD_PRIORITY                       |                     (5) | スレッドのプライオリティ(値が小さいほど優先度が低い)                                                               |
| SETTING_TASK_ASSIGNED_SIZE                    |              (4096 * 2) | スレッドのサイズ<br>スレッドの容量不足で落ちる場合は増やしてください。                                             |
| SETTING_STORAGE_SPI_FS                        |                     (1) | (1)の場合はSPIFFSに接続先情報を保持し、再起動時はそのファイルを参照する。                                          |
| SETTING_STORAGE_OVERRIDE                      |                     (0) | (1)の場合は起動時にSPIFFSの値を無視してバイナリの情報で上書きする。                                                |
| CUSHY_WEB_SERVER_OTA                          |                     (1) | (1)の場合は```http://(アドレス)/update```でOTA機能が使用可能。<br>OTAを使わない場合などに切り離せる。              |
| SETTING_WIFI_PORT                             |                    (80) | WebServerのポート番号                                                                                              |
| SETTING_WIFI_SETTING_FILE                     | "/config/info_wifi.ini" | SPIFFSの保存先パス                                                                                                 |
| SETTING_WIFI_MODE_AUTO_TRANSITIONS            |                 (false) | STAで動作中に指示されたWiFi機器が見つからない場合、自動でAPへ移行するフラグ                                        |
| SETTING_WIFI_AUTO_TRANSITIONS_DEFAULT_TIMEOUT |             (60 * 1000) | SETTING_WIFI_MODE_AUTO_TRANSITIONSが(true)の場合のWiFi機器の探索時間                                               |
| SETTING_WIFI_MODE_AP                          |                  (true) | (true)の場合はAPで起動する。(false)の場合はSTAで起動する。<br>SPIFFSにファイルがない場合に動作する                 |
| SETTING_WIFI_HOSTNAME                         |                      "" | 端末名を指定できる。空文字の場合はデフォルト値(esp32-固有番号)を使用する。<br>SPIFFSにファイルがない場合に動作する |
| SETTING_WIFI_SSID_DEFAULT                     |        "CushyWebServer" | SSIDの名前<br>SPIFFSにファイルがない場合に動作する                                                                 |
| SETTING_WIFI_PASS_DEFAULT                     |              "password" | パスワード<br>SPIFFSにファイルがない場合に動作する                                                                 |

* 接続情報について
SETTING_WIFI_SETTING_FILEで指定されたファイルパスに下記の書式で記載することも可能。

１行目：SETTING_WIFI_MODE_AP：「S」か「A」、SはSTA/AはAPで起動する
２行目：SETTING_WIFI_SSID_DEFAULT：SSID名
３行目：SETTING_WIFI_PASS_DEFAULT：パスワード
４行目：SETTING_WIFI_MODE_AUTO_TRANSITIONS：「Y」か「N」、Yだと指定されたWiFi機器がみつからないとAPに移行する
５行目：SETTING_WIFI_HOSTNAME：ホスト名を記載。空文字でもよし

記載例

```file
S
CushyWebServer
password
N
esp32-CushyWebServer
```

## Requirement

This system uses the following libraries.

* [m5stack/M5Atom](https://github.com/m5stack/M5Atom?utm_source=platformio&utm_medium=piohome)
* [fastled/FastLED](https://github.com/Makuna/NeoPixelBus?utm_source=platformio&utm_medium=piohome)
* [ayushsharma82/AsyncElegantOTA](https://github.com/ayushsharma82/AsyncElegantOTA)
* [ottowinter/ESPAsyncWebServer-esphome](https://registry.platformio.org/libraries/ottowinter/ESPAsyncWebServer-esphome)
* [ottowinter/ESPAsyncTCP-esphome](https://registry.platformio.org/libraries/ottowinter/ESPAsyncTCP-esphome)


## Changelog

It is listed [here](./Changelog).

## Notes


## Support

バグとかありましたら、下記に連絡を頂けると助かります。
[Twitter:Master_Akari](https://twitter.com/Master_Akari)

## Roadmap

Update irregularly.

<div style="display: flex">
    <div style="width:1em; background-color: red;margin-right:1em;"></div>
    <span style="">Next Update&nbsp;:&nbsp; </span>
    <span style="">No plans.</span>
</div>


## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

## Authors and acknowledgment

Show your appreciation to those who have contributed to the project.

## License

[MIT License](./LICENSE)
