# Cushy Web Server

ESP32にWebServerをたて、OTA(HTTP),AP-STA切り替えページなどを共通でつかえる部品を集めたライブラリ

## Project Status

<div style="display: flex">
    <div style="width:1em; background-color: red;margin-right:1em;"></div>
    <span style="">This project will not be refurbished unless requested.</span>
</div>

## Description



### parts list


### System  diagram


## Wiring



## Installation

This system is compiled using PlatformIO.Please install the extension from VS Code's extension recommendations. If you want to know more, check [here](https://docs.platformio.org/en/latest/).

* platformio.ini

```ini
[env:CanDataViewer-esp32can(m5stack-atom)]
platform = espressif32
board = m5stack-atom
framework = arduino
monitor_speed = 115200
lib_deps =
	m5stack/M5Atom@^0.1.0
	fastled/FastLED@^3.5.0
	https://github.com/MaSiRoProjectOSS/CANDataViewer
build_flags =
	-DLIB_CAN_DRIVER=0
	-DOUTPUT_MESSAGE_FOR_SERIAL=1
```

* main.cpp

```c++
#include <can_data_viewer.hpp>
#define SETTING_WIFI_MODE_AP false
#define SETTING_WIFI_SSID    "(wifi ssid)"
#define SETTING_WIFI_PASS    "(wifi password)"

CanDataViewer can_data_viewer;
void setup()
{
    can_data_viewer.begin();
}

void loop()
{
    (void)delay(100);
}
```

## Usage

## Requirement

This system uses the following libraries.

* [m5stack/M5Atom](https://github.com/m5stack/M5Atom?utm_source=platformio&utm_medium=piohome)
* [fastled/FastLED](https://github.com/Makuna/NeoPixelBus?utm_source=platformio&utm_medium=piohome)


## Changelog

It is listed [here](./Changelog).

## Notes


## Support


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
