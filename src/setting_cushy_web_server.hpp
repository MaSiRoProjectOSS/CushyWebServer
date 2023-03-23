/**
 * @file web_setting.hpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.23.1
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

#ifndef SETTING_STORAGE_SPI_FS
#define SETTING_STORAGE_SPI_FS (1)
#endif

#ifndef SETTING_STORAGE_OVERRIDE
#define SETTING_STORAGE_OVERRIDE (0)
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
#define SETTING_WIFI_SETTING_FILE "/config/info_wifi.ini"
#endif

#ifndef SETTING_WIFI_MODE_AUTO_TRANSITIONS
/**
 * @brief Specify Wifi mode : the system transitions to AP mode
 * when If connection is not possible for a certain period of time in STA mode.
 *
 */
#define SETTING_WIFI_MODE_AUTO_TRANSITIONS (true)
#endif

#ifndef SETTING_WIFI_AUTO_TRANSITIONS_DEFAULT_TIMEOUT
/**
 * @brief Time to wait for transition
 *
 */
#define SETTING_WIFI_AUTO_TRANSITIONS_DEFAULT_TIMEOUT (60 * 1000)
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
#define SETTING_WIFI_SSID_DEFAULT "ESP32_WEB_VIEWER"
#endif

#ifndef SETTING_WIFI_PASS_DEFAULT
/**
 * @brief Specify Wifi SSID password
 *
 */
#define SETTING_WIFI_PASS_DEFAULT "password"
#endif

#ifndef SETTING_WIFI_RECONNECTION_INTERVAL
/**
 * @brief
 *
 */
#define SETTING_WIFI_RECONNECTION_INTERVAL (1000)
#endif

const unsigned long THREAD_WEB_RECONNECTION_INTERVAL_WIFI = SETTING_WIFI_RECONNECTION_INTERVAL;

#endif
