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

namespace MaSiRoProject
{
namespace Web
{
//////////////////////////////////////////////////////////////
// WEB server settings
//////////////////////////////////////////////////////////////

#ifndef SETTING_DEFAULT_FAVICON
#define SETTING_DEFAULT_FAVICON (1)
#endif

//////////////////////////////////////////////////////////////
// SNTP settings
//////////////////////////////////////////////////////////////

#ifndef SETTING_SNTP_ENABLE
#define SETTING_SNTP_ENABLE (0)
#endif

#ifndef SETTING_SNTP_SERVER
#define SETTING_SNTP_SERVER "pool.ntp.org"
#endif

#ifndef SETTING_SNTP_TIME_ZONE
#define SETTING_SNTP_TIME_ZONE "UTC"
#endif

//////////////////////////////////////////////////////////////
// OTA settings
//////////////////////////////////////////////////////////////

#ifndef CUSHY_WEB_SERVER_OTA
#define CUSHY_WEB_SERVER_OTA (1)
#endif

//////////////////////////////////////////////////////////////
// Server settings
//////////////////////////////////////////////////////////////

#ifndef SETTING_WIFI_PORT
/**
 * @brief Wifi port
 *
 */
#define SETTING_WIFI_PORT (80)
#endif

#ifndef SETTING_WIFI_HOSTNAME
/**
 * @brief Wifi HOSTNAME
 *
 */
#define SETTING_WIFI_HOSTNAME ""
#endif

#ifndef SETTING_WIFI_SETTING_FILE
/**
 * @brief Wifi settings file path
 *
 */
#define SETTING_WIFI_SETTING_FILE "/config/wifi_settings.ini"
#endif

//////////////////////////////////////////////////////////////
// WIFI settings : AP mode
//////////////////////////////////////////////////////////////

#ifndef SETTING_WIFI_AP_DEFAULT_ENABLE
/**
 * @brief Wifi mode : AP mode
 *
 */
#define SETTING_WIFI_AP_DEFAULT_ENABLE (true)
#endif

#ifndef SETTING_WIFI_AP_SETTING_FILE
/**
 * @brief Wifi settings file path : AP mode
 *
 */
#define SETTING_WIFI_AP_SETTING_FILE "/config/wifi_ap.ini"
#endif

#ifndef SETTING_WIFI_AP_DEFAULT_SSID
/**
 * @brief Wifi SSID : AP mode
 *
 */
#define SETTING_WIFI_AP_DEFAULT_SSID "CushyWebServer"
#endif

#ifndef SETTING_WIFI_AP_DEFAULT_PASSWORD
/**
 * @brief Wifi password : AP mode
 *
 */
#define SETTING_WIFI_AP_DEFAULT_PASSWORD "password!"
#endif

//////////////////////////////////////////////////////////////
// WIFI settings : STA mode
//////////////////////////////////////////////////////////////

#ifndef SETTING_WIFI_STA_DEFAULT_ENABLE
/**
 * @brief Wifi mode : STA mode
 *
 */
#define SETTING_WIFI_STA_DEFAULT_ENABLE (false)
#endif

#ifndef SETTING_WIFI_STA_CONNECTED_FILE
/**
 * @brief Wifi settings file path : STA mode
 *
 */
#define SETTING_WIFI_STA_CONNECTED_FILE "/config/wifi_sta.ini"
#endif

#ifndef SETTING_WIFI_STA_FILE_PATTERN
/**
 * @brief WiFi configuration file list pattern
 *
 */
#define SETTING_WIFI_STA_FILE_PATTERN "/config/wifi_sta_%02d.ini"
#endif

#ifndef SETTING_WIFI_STA_FILE_MAX
/**
 * @brief Number of WiFi configuration file list
 *
 */
#define SETTING_WIFI_STA_FILE_MAX (5)
#endif

#ifndef SETTING_WIFI_STA_AUTO_TRANSITIONS_TIMEOUT
/**
 * @brief Time to wait for transition [unit:s]
 *
 */
#define SETTING_WIFI_STA_AUTO_TRANSITIONS_TIMEOUT (60)
#endif

#ifndef SETTING_WIFI_STA_DEFAULT_SSID
/**
 * @brief Wifi SSID : STA mode
 *
 */
#define SETTING_WIFI_STA_DEFAULT_SSID "CushyWebServer"
#endif

#ifndef SETTING_WIFI_STA_DEFAULT_PASSWORD
/**
 * @brief Wifi password : STA mode
 *
 */
#define SETTING_WIFI_STA_DEFAULT_PASSWORD "password!"
#endif

//////////////////////////////////////////////////////////////
// SPIFFS settings
//////////////////////////////////////////////////////////////

#ifndef SETTING_WIFI_STORAGE_SPI_FORMAT
#define SETTING_WIFI_STORAGE_SPI_FORMAT (true)
#endif

#ifndef SETTING_WIFI_STORAGE_SPI_FS
#define SETTING_WIFI_STORAGE_SPI_FS (1)
#endif

#ifndef SETTING_WIFI_STORAGE_OVERRIDE
#define SETTING_WIFI_STORAGE_OVERRIDE (0)
#endif

//////////////////////////////////////////////////////////////
// Thread settings
//////////////////////////////////////////////////////////////
// Task assigned
#ifndef SETTING_THREAD_TASK_ASSIGNED_SIZE_WIFI
#define SETTING_THREAD_TASK_ASSIGNED_SIZE_WIFI (4096 * 3)
#endif
#ifndef SETTING_THREAD_TASK_ASSIGNED_SIZE_SERVER
#define SETTING_THREAD_TASK_ASSIGNED_SIZE_SERVER (4096 * 3)
#endif
// PRIORITY (0(LOW) - 25(height)))
#ifndef SETTING_THREAD_PRIORITY_SERVER
#define SETTING_THREAD_PRIORITY_SERVER (5)
#endif
#ifndef SETTING_THREAD_PRIORITY_WIFI
#define SETTING_THREAD_PRIORITY_WIFI (2)
#endif

// CORE
#ifndef SETTING_THREAD_CORE_CUSHY_WEB_SERVER
#define SETTING_THREAD_CORE_CUSHY_WEB_SERVER (1)
#endif

} // namespace Web
} // namespace MaSiRoProject
#endif
