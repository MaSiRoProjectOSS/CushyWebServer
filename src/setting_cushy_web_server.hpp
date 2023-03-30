/**
 * @file web_setting.hpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.0.1
 * @date 2023-03-12
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#ifndef WEB_SETTING_HPP
#define WEB_SETTING_HPP

#ifndef SETTING_THREAD_CORE_WIFI
#define SETTING_THREAD_CORE_WIFI (1)
#endif
#ifndef SETTING_THREAD_PRIORITY
#define SETTING_THREAD_PRIORITY (5)
#endif
#ifndef SETTING_THREAD_TASK_ASSIGNED_SIZE
#define SETTING_THREAD_TASK_ASSIGNED_SIZE (4096 * 2)
#endif

#ifndef SETTING_WIFI_STORAGE_SPI_FS
#define SETTING_WIFI_STORAGE_SPI_FS (1)
#endif

#ifndef SETTING_WIFI_STORAGE_OVERRIDE
#define SETTING_WIFI_STORAGE_OVERRIDE (0)
#endif

#ifndef CUSHY_WEB_SERVER_OTA
#define CUSHY_WEB_SERVER_OTA (1)
#endif
#ifndef SETTING_DEFAULT_FAVICON
#define SETTING_DEFAULT_FAVICON (1)
#endif

#ifndef SETTING_WIFI_PORT
/**
 * @brief Specify Wifi port
 *
 */
#define SETTING_WIFI_PORT (80)
#endif

#ifndef SETTING_WIFI_SETTING_FILE
/**
 * @brief Specify Wifi settings file path
 *
 */
#define SETTING_WIFI_SETTING_FILE "/config/wifi.ini"
#endif

#ifndef SETTING_WIFI_MODE_AUTO_TRANSITIONS
/**
 * @brief Specify Wifi mode : the system transitions to AP mode
 * when If connection is not possible for a certain period of time in STA mode.
 *
 */
#define SETTING_WIFI_MODE_AUTO_TRANSITIONS (true)
#endif

#ifndef SETTING_WIFI_SETTING_LIST_FILE
/**
 * @brief WiFi configuration file list pattern
 *
 */
#define SETTING_WIFI_SETTING_LIST_FILE "/config/wifi_%02d.ini"
#endif

#ifndef SETTING_WIFI_SETTING_LIST_MAX
/**
 * @brief Number of WiFi configuration file list
 *
 */
#define SETTING_WIFI_SETTING_LIST_MAX (10)
#endif

#ifndef SETTING_WIFI_AUTO_TRANSITIONS_DEFAULT_TIMEOUT
/**
 * @brief Time to wait for transition [unit:s]
 *
 */
#define SETTING_WIFI_AUTO_TRANSITIONS_DEFAULT_TIMEOUT (60)
#endif

#ifndef SETTING_WIFI_MODE_AP
/**
 * @brief Specify Wifi mode : Access point
 *
 */
#define SETTING_WIFI_MODE_AP (true)
#endif

#ifndef SETTING_WIFI_HOSTNAME
/**
 * @brief Specify Wifi HOSTNAME
 *
 */
//#define SETTING_WIFI_HOSTNAME "ESP32_DEVICE"
#endif

#ifndef SETTING_WIFI_SSID_DEFAULT
/**
 * @brief Specify Wifi SSID
 *
 */
#define SETTING_WIFI_SSID_DEFAULT "CushyWebServer"
#endif

#ifndef SETTING_WIFI_PASS_DEFAULT
/**
 * @brief Specify Wifi SSID password
 *
 */
#define SETTING_WIFI_PASS_DEFAULT "password!"
#endif

#ifndef SETTING_DEFAULT_FAVICON
#define SETTING_DEFAULT_FAVICON (1)
#endif

#ifndef SETTING_SNTP_ENABLE
#define SETTING_SNTP_ENABLE (1)
#endif
#ifndef SETTING_SNTP_SERVER
#define SETTING_SNTP_SERVER "pool.ntp.org"
#endif

#ifndef SETTING_SNTP_TIME_ZONE
#define SETTING_SNTP_TIME_ZONE "UTC"
#endif

#endif
