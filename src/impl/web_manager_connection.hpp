/**
 * @file web_manager_connection.hpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.0.1
 * @date 2023-03-12
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#ifndef MASIRO_PROJECT_WEB_MANAGER_CONNECTION_HPP
#define MASIRO_PROJECT_WEB_MANAGER_CONNECTION_HPP

#include "web_manager_setting.hpp"

#include <WiFi.h>
#include <vector>

namespace MaSiRoProject
{
namespace Web
{
class WebManagerConnection : public WebManagerSetting {
public:
    class NetworkList {
    public:
        String name;                 //!< SSID
        int rssi;                    //!< RSSI
        int quality;                 //!< RSSI as Quality
        wifi_auth_mode_t encryption; //!< Encryption Type

    public:
        /**
     * @brief Sort by RSSI
     *
     * @param left
     * @param right
     * @return true
     * @return false
     */
        static bool compar_rssi(const NetworkList &left, const NetworkList &right)
        {
            return left.rssi > right.rssi;
        }
    };

private:
    class config_address {
    public:
        IPAddress local_ip = INADDR_NONE;
        IPAddress gateway  = INADDR_NONE;
        IPAddress subnet   = INADDR_NONE;
        bool flag_set      = false;
    };

public:
    WebManagerConnection();

public:
    bool begin();
    bool reconnect(bool save);
    bool reconnect_default(bool save);
    bool reconnect(std::string ssid, std::string pass, bool ap_mode, bool auto_default, bool save);
    bool disconnect();

public:
    void config_address_ap(IPAddress ip, IPAddress subnet, IPAddress gateway = INADDR_NONE);
    void config_address_sta(IPAddress ip, IPAddress subnet, IPAddress gateway = INADDR_NONE);

public:
    bool is_connected(bool immediate = true);
    bool is_ap_mode();
    const char *get_ssid();
    IPAddress get_ip();
    std::vector<NetworkList> get_wifi_list(int *length);
    bool make_wifi_list();

private:
    int _get_rssi_as_quality(int rssi);
    bool _reconnect(std::string ssid, std::string pass, bool ap_mode, bool auto_default, bool save);

private:
    bool _running = false;
    config_address _config_ap;
    config_address _config_sta;
    enum MODE_CONNECTION
    {
        MODE_CONNECTION_NONE,
        MODE_CONNECTION_AP,
        MODE_CONNECTION_STA,
    };
    MODE_CONNECTION _mode = MODE_CONNECTION::MODE_CONNECTION_NONE;

    bool _flag_make_net_list = false;

private:
    const int CONNECTION_TIMEOUT_MS       = (5 * 1000);
    const int CONNECTION_TIMEOUT_INTERVAL = (100);
    const int CONNECTION_CHECK_INTERVAL   = (5 * 1000);
};
} // namespace Web
} // namespace MaSiRoProject
#endif
