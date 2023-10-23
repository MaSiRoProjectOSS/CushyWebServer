# Cushy Web Server

ESP32に下記の機能を持たせるためのライブラリ

* WebServer
  * OTA(HTTP)
  * WiFi切り替えページ
  * HttpClientによるWebAPI
  * SNTPによる時刻同期

## Project Status

<div style="display: flex">
    <div style="width:1em; background-color: red;margin-right:1em;"></div>
    <span style="">This project will not be refurbished unless requested.</span>
</div>

## Description

CushyWebServer クラスを継承する事でESP32(M5Stack)で動作する WebServerを容易に立てるライブラリです。
ページの追加方法は、[ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) をご参照ください。

関数```cushy.begin()```を実行することで、Core1にスレッドを立ててネットワーク接続まで実施する。

[ayushsharma82/ElegantOTA](https://github.com/ayushsharma82/ElegantOTA)を使用しおりURL```http://(アドレス)/update```でバイナリの書き換えが行える。

ネットワーク切り替えページ```http://(アドレス)/network```を持っており、接続先をブラウザから切り替える事ができる。

このライブラリの設定は```src\setting_cushy_web_server.hpp```で管理しており、```plantformio.ini```の```build_flags```で制御することを想定している。

接続先情報はSPIFFSに持っており以下の順番で接続を確認します。

1. ```SETTING_WIFI_STA_FILE_PATTERN```で設定されたルールに従った[接続情報ファイル](#接続情報ファイルについて)
   * データのアップロードはPlatformIOの[Uploading files to Filesystem](https://docs.platformio.org/en/latest/platforms/espressif8266.html#uploading-files-to-filesystem)を参照してださい。
1. [Defineによる設定](#Defineによる設定)で定義した```SETTING_WIFI_SSID_DEFAULT``` / ```SETTING_WIFI_PASS_DEFAULT``` / ```SETTING_WIFI_MODE_AP``` / ```SETTING_WIFI_HOSTNAME``` に従う

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
build_flags =
	-D ELEGANTOTA_USE_ASYNC_WEBSERVER=1
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

### Defineによる設定

設定ファイル```src\setting_cushy_web_server.hpp```で定義したdefineは下記の通りです。

#### オリジナルFaviconのデータ設定

| define                  | デフォルト値 | 定義                                                                                      |
| :---------------------- | -----------: | :---------------------------------------------------------------------------------------- |
| SETTING_DEFAULT_FAVICON |          (1) | (1)の場合は本プロジェクトのFaviconを使用する。容量が逼迫している場合は(0)にしてください。 |

#### SNTP設定

| define                 |   デフォルト値 | 定義                        |
| :--------------------- | -------------: | :-------------------------- |
| SETTING_SNTP_ENABLE    |            (0) | (1)の場合はSNTPを有効にする |
| SETTING_SNTP_SERVER    | "pool.ntp.org" | 接続するSNTPサーバ名        |
| SETTING_SNTP_TIME_ZONE |          "UTC" | タイムゾーン                |

#### OTA設定

| define               | デフォルト値 | 定義                                                                                                  |
| :------------------- | -----------: | :---------------------------------------------------------------------------------------------------- |
| CUSHY_WEB_SERVER_OTA |          (1) | (1)の場合は```http://(アドレス)/update```でOTA機能が使用可能。<br>OTAを使わない場合などに切り離せる。 |

#### WebServer 設定

| define                | デフォルト値 | 定義                                                                       |
| :-------------------- | -----------: | :------------------------------------------------------------------------- |
| SETTING_WIFI_PORT     |         (80) | WebServerのポート番号                                                      |
| SETTING_WIFI_HOSTNAME |           "" | 端末名を指定できる。空文字の場合はデフォルト値(esp32-固有番号)を使用する。 |

#### WiFi APモード接続設定

| define                           |          デフォルト値 | 定義                                               |
| :------------------------------- | --------------------: | :------------------------------------------------- |
| SETTING_WIFI_AP_DEFAULT_ENABLE   |                (true) | APモードで動作させる                               |
| SETTING_WIFI_AP_SETTING_FILE     | "/config/wifi_ap.ini" | APモードのWiFiの接続情報の保存先                   |
| SETTING_WIFI_AP_DEFAULT_SSID     |      "CushyWebServer" | SSIDの名前<br>SPIFFSにファイルがない場合に動作する |
| SETTING_WIFI_AP_DEFAULT_PASSWORD |           "password!" | パスワード<br>SPIFFSにファイルがない場合に動作する |

#### WiFi STAモード接続設定

| define                                    |                デフォルト値 | 定義                                                                                                                        |
| :---------------------------------------- | --------------------------: | :-------------------------------------------------------------------------------------------------------------------------- |
| SETTING_WIFI_STA_DEFAULT_ENABLE           |                     (false) | STAモードで動作させる                                                                                                       |
| SETTING_WIFI_STA_CONNECTED_FILE           |      "/config/wifi_sta.ini" | STAモードで最後に接続成功した接続情報の保存先                                                                               |
| SETTING_WIFI_STA_FILE_PATTERN             | "/config/wifi_sta_%02d.ini" | SETTING_WIFI_MODE_AUTO_TRANSITIONSが(true)の場合に参照するWiFiリスト。indexは0～SETTING_WIFI_SETTING_LIST_MAXを参照する (*) |
| SETTING_WIFI_STA_FILE_MAX                 |                         (5) | SETTING_WIFI_SETTING_LIST_FILEが許容するファイル数                                                                          |
| SETTING_WIFI_STA_AUTO_TRANSITIONS_TIMEOUT |                        (60) | SETTING_WIFI_MODE_AUTO_TRANSITIONSが(true)の場合のWiFi機器の探索時間[単位:秒,5秒以上推奨]                                   |
| SETTING_WIFI_STA_LOOP_FILE                |                      (true) | 接続先がない場合は、ファイルリストの最初から探査する                                                                        |
| SETTING_WIFI_STA_DEFAULT_SSID             |            "CushyWebServer" | SSIDの名前<br>SPIFFSにファイルがない場合に動作する                                                                          |
| SETTING_WIFI_STA_DEFAULT_PASSWORD         |                 "password!" | パスワード<br>SPIFFSにファイルがない場合に動作する                                                                          |

*) SETTING_WIFI_SETTING_LIST_FILEが"/config/wifi_%02d.ini"の場合、/config/wifi_00.ini～/config/wifi_04.iniまでのファイルを参照する。

#### WiFiの保存設定接続設定

| define                          | デフォルト値 | 定義                                                                            |
| :------------------------------ | -----------: | :------------------------------------------------------------------------------ |
| SETTING_WIFI_STORAGE_OVERRIDE   |          (0) | (1)の場合は起動時にSPIFFSの値を無視してバイナリの情報で接続先情報を上書きする。 |
| SETTING_WIFI_STORAGE_SPI_FS     |          (1) | (1)の場合はSPIFFSに接続先情報を保持し、再起動時はそのファイルを参照する。       |
| SETTING_WIFI_STORAGE_SPI_FORMAT |       (true) | (1)の場合はSPIFFSがフォーマットされてない場合はフォーマットする                 |

#### スレッド動作設定

| define                            | デフォルト値 | 定義                                                                   |
| :-------------------------------- | -----------: | :--------------------------------------------------------------------- |
| SETTING_THREAD_CORE_WIFI          |          (1) | スレッドを動作させているCore番号                                       |
| SETTING_THREAD_PRIORITY           |          (5) | スレッドのプライオリティ(値が小さいほど優先度が低い)                   |
| SETTING_THREAD_TASK_ASSIGNED_SIZE |   (4096 * 2) | スレッドのサイズ<br>スレッドの容量不足で落ちる場合は増やしてください。 |


## Requirement

This system uses the following libraries.

* [m5stack/M5Atom](https://github.com/m5stack/M5Atom?utm_source=platformio&utm_medium=piohome)
* [fastled/FastLED](https://github.com/Makuna/NeoPixelBus?utm_source=platformio&utm_medium=piohome)
* [ayushsharma82/ElegantOTA](https://github.com/ayushsharma82/ElegantOTA)
* [ottowinter/ESPAsyncWebServer-esphome](https://registry.platformio.org/libraries/ottowinter/ESPAsyncWebServer-esphome)
* [ottowinter/ESPAsyncTCP-esphome](https://registry.platformio.org/libraries/ottowinter/ESPAsyncTCP-esphome)
* [bblanchon/ArduinoJson](https://github.com/bblanchon/ArduinoJson)
* [suculent/AESLib](https://github.com/suculent/thinx-aes-lib)


## Changelog

It is listed [here](./Changelog).

## Notes

* M5Atom Liteの場合はAPモードで動作するとデバイスが熱を持つ現象を確認しています。

## Support

バグとかありましたら、下記に連絡を頂けると助かります。
[Twitter:Master_Akari](https://twitter.com/Master_Akari)

## Roadmap

* ファイルサイズのスリム化
  * Ver.4.0以降は、M5ATOMだと容量圧迫により書き込み不可能（OTAなど削りHuge_appのみ動作）
* Network ページ
  * AP/STAの有効無効が設定出来るようにする。
    * 両方を閉じるとアクセス不可になるため、どちらかが有効になるようにする。
      * AP
      * STA
      * AP and STA
  * STAは複数の接続優先度を持てるため、それを選択可能にする。
* URL設定
  * 公開しているＵＲＬは固定値のため、設定で変更可能にする。（ビルド時のみ）


<!--
Update irregularly.

<div style="display: flex">
    <div style="width:1em; background-color: red;margin-right:1em;"></div>
    <span style="">Next Update&nbsp;:&nbsp; </span>
    <span style="">No plans.</span>
</div>
-->


## Contributing

We welcome pull requests from the community. If you're considering significant changes, we kindly ask you to begin by opening an issue to initiate a discussion about your proposed modifications.
Additionally, when submitting a pull request, please ensure that any relevant tests are updated or added as needed.

## Authors and acknowledgment

We offer heartfelt thanks to the open-source community for the invaluable gifts they've shared with us. The hardware, libraries, and tools they've provided have breathed life into our journey of development. Each line of code and innovation has woven a tapestry of brilliance, lighting our path. In this symphony of ingenuity, we find ourselves humbled and inspired. These offerings infuse our project with boundless possibilities. As we create, they guide us like stars, reminding us that collaboration can turn dreams into reality. With deep appreciation, we honor the open-source universe that nurtures us on this journey of discovery and growth.

## License

[MIT License](./LICENSE)


---

<!--
Apply styles when markdown
-->

<style>
  body {
    counter-reset: chapter;
}

h2 {
    counter-reset: sub-chapter;
}

h3 {
    counter-reset: section;
}

h4 {
    counter-reset: indexlist;
}

h1::before {
    counter-reset: chapter;
}

h2::before {
    counter-increment: chapter;
    content: counter(chapter) ". ";
}

h3::before {
    counter-increment: sub-chapter;
    content: counter(chapter) "-" counter(sub-chapter) ". ";
}

h4::before {
    counter-increment: section;
    content: counter(chapter) "-" counter(sub-chapter) "-" counter(section) ". ";
}

h5::before {
    counter-increment: indexlist;
    content: "(" counter(indexlist) ") ";
}

#sidebar-toc-btn {
    bottom: unset;
    top: 8px;
}

.markdown-preview.markdown-preview {
    h2 {
        border-bottom: 4px solid #eaecef;
    }

    h3 {
        border-bottom: 1px solid #eaecef;
    }
}

.md-sidebar-toc.md-sidebar-toc {
    padding-top: 40px;
}

#sidebar-toc-btn {
    bottom: unset;
    top: 8px;
}
</style>
