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

namespace MaSiRoProject
{
namespace Web
{
class WebManagerSetting {
public:
    WebManagerSetting();

public:
    bool save_information_ap(std::string ssid, std::string pass, std::string hostname);
    bool load_information_ap();

public:
    bool save_information_sta(std::string ssid, std::string pass, std::string hostname, int num);
    void set_hostname(std::string hostname);
    bool load_sta_settings(bool clear);

protected:
    bool _setup();

protected:
    std::string _hostname;
    std::string _sta_ssid;
    std::string _sta_pass;
    std::string _ap_ssid;
    std::string _ap_pass;
    int _sta_explored_index = 0;

    void set_information_ap(std::string ssid, std::string pass, std::string hostname);
    void set_information_sta(std::string ssid, std::string pass, std::string hostname);

private:
    bool _default_information_ap();
    void _set_information_ap(std::string ssid, std::string pass, std::string hostname);

private:
    bool _default_information_sta();
    void _set_information_sta(std::string ssid, std::string pass, std::string hostname);
    bool _load_sta_setting(bool clear);

private:
    bool _load_information(std::string file, bool mode_ap);
    bool _save_information(std::string file, std::string ssid, std::string pass, std::string hostname);

private:
    bool _open_fs;
    int _error_count_spi = 3;

protected:
    bool _connect_ap  = SETTING_WIFI_AP_DEFAULT_ENABLE;
    bool _connect_sta = SETTING_WIFI_STA_DEFAULT_ENABLE;
};

} // namespace Web
} // namespace MaSiRoProject
#endif
