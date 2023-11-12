/**
 * @file web_manager_setting.hpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.0.1
 * @date 2023-03-12
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#ifndef MASIRO_PROJECT_WEB_MANAGER_SETTING_HPP
#define MASIRO_PROJECT_WEB_MANAGER_SETTING_HPP

#include "../setting_cushy_web_server.hpp"

#include <Arduino.h>
#if SETTING_WIFI_STORAGE_SPI_FS
#include <SPIFFS.h>
#else
#include <FS.h>
#endif

namespace MaSiRoProject
{
namespace Web
{
class WebManagerSetting {
public:
    class SettingsInformation {
    public:
        std::string hostname = "";
        std::string ssid     = "";
        std::string pass     = "";
        bool result          = false;
    };

public:
    WebManagerSetting();

public:
    void set_hostname(std::string hostname);
    int get_sta_list_selected();
    std::string get_sta_list_ssid(int index);
    std::string get_sta_list_hostname(int index);

public:
    //////////////////////////////////////////////////////////////
    // AP settings
    //////////////////////////////////////////////////////////////
    bool save_ap_setting(bool enable, std::string ssid, std::string pass);
    bool save_ap_information(std::string ssid, std::string pass, std::string hostname);
    bool load_ap_settings();
    bool set_ap_enable(bool flag);

public:
    //////////////////////////////////////////////////////////////
    // STA settings
    //////////////////////////////////////////////////////////////
    bool save_sta_setting(bool enable, std::string ssid, std::string pass, std::string hostname, int num);
    bool save_sta_information(std::string ssid, std::string pass, std::string hostname, int num);
    bool load_sta_settings(bool clear);
    bool set_sta_enable(bool flag);

protected:
    std::string _hostname;
    std::string _sta_ssid;
    std::string _sta_pass;
    std::string _ap_ssid;
    std::string _ap_pass;
    int _sta_explored_index = 0;
    bool _connect_ap        = SETTING_WIFI_AP_DEFAULT_ENABLE;
    bool _connect_sta       = SETTING_WIFI_STA_DEFAULT_ENABLE;
    bool _enable_ap         = false;
    bool _enable_sta        = false;

    bool _setup();
    void set_ap_information(std::string ssid, std::string pass);
    void set_sta_information(std::string ssid, std::string pass);

    int _sta_list_selected                                    = -1;
    std::string _sta_list_ssid[SETTING_WIFI_STA_FILE_MAX]     = { "" };
    std::string _sta_list_hostname[SETTING_WIFI_STA_FILE_MAX] = { "" };

private:
    //////////////////////////////////////////////////////////////
    // WiFi settings
    //////////////////////////////////////////////////////////////
    bool _load_settings_wifi(fs::FS &fs);
    bool _save_settings_wifi(fs::FS &fs, bool ap_mode, bool sta_mode);

private:
    void _init_sta_setting(fs::FS &fs);
    bool _load_sta_setting(fs::FS &fs, bool clear);
    bool _load_information(fs::FS &fs, std::string file, bool mode_ap);
    bool _save_information(fs::FS &fs, std::string file, std::string ssid, std::string pass, std::string hostname);

private:
    bool _open_fs;
    int _error_count_spi;

private:
    int ERROR_COUNT_SPI_MAX = 3;
};

} // namespace Web
} // namespace MaSiRoProject
#endif
