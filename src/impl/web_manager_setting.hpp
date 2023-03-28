/**
 * @file web_manager_setting.hpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.23.1
 * @date 2023-03-12
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#ifndef MASIRO_PROJECT_WEB_MANAGER_SETTING_HPP
#define MASIRO_PROJECT_WEB_MANAGER_SETTING_HPP

#include <Arduino.h>

namespace MaSiRoProject
{
namespace Web
{
class WebManagerSetting {
public:
    WebManagerSetting();

public:
    bool save_information(std::string ssid, std::string pass, bool ap_mode, bool auto_default);
    bool load_information();
    void set_hostname(std::string hostname);
    bool load_auto_setting(bool clear);

protected:
    bool _setup();

protected:
    std::string _hostname;
    bool _auto_default_setting;
    bool _mode_ap;
    std::string _ssid;
    std::string _pass;
    int _explored_index = 0;

    void set_information(std::string ssid, std::string pass, bool ap_mode, bool auto_default);

private:
    bool _default_information();
    void _set_information(std::string ssid, std::string pass, bool ap_mode, bool auto_default);
    bool _load_information(std::string file);
    bool _save_information(std::string ssid, std::string pass, bool ap_mode, bool auto_default);
    bool _load_auto_setting(bool clear);

private:
    bool _open_fs;
    int _error_count_spi = 3;
};

} // namespace Web
} // namespace MaSiRoProject
#endif
