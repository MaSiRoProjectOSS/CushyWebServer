/**
 * @file web_manager_connection.cpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.23.1
 * @date 2023-03-12
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#include "web_manager_connection.hpp"

#include "../setting_cushy_web_server.hpp"

namespace MaSiRoProject
{
namespace Web
{
WebManagerConnection::WebManagerConnection()
{
}

////////////////////////////////////////////////////
// config function
////////////////////////////////////////////////////
void WebManagerConnection::config_address_ap(IPAddress ip, IPAddress subnet, IPAddress gateway)
{
    if (INADDR_NONE != ip) {
        if (INADDR_NONE != subnet) {
            this->_config_ap.flag_set = true;
            this->_config_ap.local_ip = ip;
            this->_config_ap.subnet   = subnet;
            this->_config_ap.gateway  = gateway;
        }
    }
}
void WebManagerConnection::config_address_sta(IPAddress ip, IPAddress subnet, IPAddress gateway)
{
    if (INADDR_NONE != ip) {
        if (INADDR_NONE != subnet) {
            this->_config_sta.flag_set = true;
            this->_config_sta.local_ip = ip;
            this->_config_sta.subnet   = subnet;
            this->_config_sta.gateway  = gateway;
        }
    }
}

////////////////////////////////////////////////////
// information function
////////////////////////////////////////////////////

bool WebManagerConnection::make_wifi_list()
{
    bool result                       = false;
    static unsigned long release_time = 0;
    unsigned long current             = millis();

    if (release_time < current) {
        static unsigned long RELEASE_INTERVAL = (60 * 1000);

        this->_flag_make_net_list = false;
        release_time              = current + RELEASE_INTERVAL;
    }

    if (false == this->_flag_make_net_list) {
        if (WIFI_SCAN_FAILED != WiFi.scanNetworks(true)) {
            this->_flag_make_net_list = true;
            result                    = true;
        }
    }
    return result;
}

std::vector<WebManagerConnection::NetworkList> WebManagerConnection::get_wifi_list(int *length)
{
    std::vector<NetworkList> list;
    if (true == this->_flag_make_net_list) {
        *length = WiFi.scanComplete();
        if (0 <= *length) {
            for (int i = 0; i < *length; i++) {
                NetworkList item;
                String name = WiFi.SSID(i);
                if (0 != name.length()) {
                    item.name       = name;
                    item.encryption = WiFi.encryptionType(i);
                    item.rssi       = WiFi.RSSI(i);
                    item.quality    = this->_get_rssi_as_quality(item.rssi);
                    list.push_back(item);
                }
            }
            if (1 < *length) {
                std::sort(list.begin(), list.end(), NetworkList::compar_rssi);
            }
            this->_flag_make_net_list = false;
        } else if (WIFI_SCAN_FAILED == *length) {
            this->_flag_make_net_list = false;
        }
    }
    return list;
}
IPAddress WebManagerConnection::get_ip()
{
    wifi_mode_t mode = WiFi.getMode();
    switch (mode) {
        case wifi_mode_t::WIFI_MODE_AP:
            return WiFi.softAPIP();
        case wifi_mode_t::WIFI_MODE_STA:
            return WiFi.localIP();
        case wifi_mode_t::WIFI_MODE_APSTA:
        default:
            return INADDR_NONE;
    }
}
const char *WebManagerConnection::get_ssid()
{
    wifi_mode_t mode = WiFi.getMode();
    switch (mode) {
        case wifi_mode_t::WIFI_MODE_AP:
            return WiFi.softAPSSID().c_str();
        case wifi_mode_t::WIFI_MODE_STA:
            return WiFi.SSID().c_str();
        case wifi_mode_t::WIFI_MODE_APSTA:
        default:
            return "--";
    }
}
bool WebManagerConnection::is_ap_mode()
{
    bool result      = false;
    wifi_mode_t mode = WiFi.getMode();
    switch (mode) {
        case wifi_mode_t::WIFI_MODE_AP:
        case wifi_mode_t::WIFI_MODE_APSTA:
            result = true;
            break;
        default:
            break;
    }
    return result;
}

bool WebManagerConnection::is_connected(bool immediate)
{
    static unsigned long next_time = 0;
    static bool result             = false;
    if ((true == immediate) || (next_time < millis())) {
        next_time        = millis() + this->CONNECTION_CHECK_INTERVAL;
        wifi_mode_t mode = WiFi.getMode();
        wl_status_t status;
        switch (mode) {
            case wifi_mode_t::WIFI_MODE_STA:
                status = WiFi.status();
                switch (status) {
                    case wl_status_t::WL_CONNECTED:
                    case wl_status_t::WL_SCAN_COMPLETED:
                    case wl_status_t::WL_IDLE_STATUS:
                        result = true;
                        break;
                    case wl_status_t::WL_CONNECTION_LOST:
                    case wl_status_t::WL_CONNECT_FAILED:
                    case wl_status_t::WL_NO_SSID_AVAIL:
                    case wl_status_t::WL_NO_SHIELD:
                    case wl_status_t::WL_DISCONNECTED:
                    default:
                        result = false;
                        break;
                }
                break;

            case wifi_mode_t::WIFI_MODE_AP:
            default:
                result = true;
                break;
        }
    }
    return result;
}

////////////////////////////////////////////////////
// connection function
////////////////////////////////////////////////////
bool WebManagerConnection::begin()
{
#ifdef SETTING_WIFI_HOSTNAME
    this->set_hostname(SETTING_WIFI_HOSTNAME);
#endif
    return this->_setup();
}

bool WebManagerConnection::reconnect(bool save)
{
    return this->reconnect(this->_ssid, this->_pass, this->_mode_ap, this->_auto_default_setting, save);
}
bool WebManagerConnection::reconnect_default(bool save)
{
    return this->reconnect(SETTING_WIFI_SSID_DEFAULT, SETTING_WIFI_PASS_DEFAULT, SETTING_WIFI_MODE_AP, SETTING_WIFI_MODE_AUTO_TRANSITIONS, save);
}

bool WebManagerConnection::reconnect(std::string ssid, std::string pass, bool ap_mode, bool auto_default, bool save)
{
    if ("" == ssid) {
        ssid = this->_ssid;
    }
    if ("" == pass) {
        pass = this->_pass;
    }
    bool result = this->_reconnect(ssid, pass, ap_mode, auto_default, save);
    if (false == result) {
        if ((this->_ssid != ssid) || (this->_pass != pass) || (this->_mode_ap != ap_mode) || (this->_auto_default_setting != auto_default)) {
            result = this->_reconnect(this->_ssid, this->_pass, this->_mode_ap, this->_auto_default_setting, false);
        }
    }
    if (true == result) {
        this->_explored_index = 0;
    }
    return result;
}

bool WebManagerConnection::disconnect()
{
    bool result      = true;
    wifi_mode_t mode = WiFi.getMode();
    switch (mode) {
        case wifi_mode_t::WIFI_MODE_AP:
            result = WiFi.softAPdisconnect();
            break;
        case wifi_mode_t::WIFI_MODE_STA:
            if (true == WiFi.isConnected()) {
                result = WiFi.disconnect();
            }
            break;

        case wifi_mode_t::WIFI_MODE_APSTA:
            result = WiFi.softAPdisconnect();
            if (true == WiFi.isConnected()) {
                result = WiFi.disconnect();
            }
        default:
            break;
    }
    if (false == result) {
        int countdown = this->CONNECTION_TIMEOUT_MS;
        while (true == this->is_connected(true)) {
            delay(this->CONNECTION_TIMEOUT_INTERVAL);
            countdown -= this->CONNECTION_TIMEOUT_INTERVAL;
            if (0 >= countdown) {
                break;
            }
        }
    }
    result = !this->is_connected(true);
    if (true == result) {
        this->_mode = MODE_CONNECTION::MODE_CONNECTION_NONE;
    }
    return result;
}

////////////////////////////////////////////////////
// private function
////////////////////////////////////////////////////
int WebManagerConnection::_get_rssi_as_quality(int rssi)
{
    int quality = 0;

    if (rssi <= -100) {
        quality = 0;
    } else if (rssi >= -50) {
        quality = 100;
    } else {
        quality = 2 * (rssi + 100);
    }
    return quality;
}

bool WebManagerConnection::_reconnect(std::string ssid, std::string pass, bool ap_mode, bool auto_default, bool save)
{
    bool result = true;
    (void)this->disconnect();
    if (MODE_CONNECTION::MODE_CONNECTION_NONE == this->_mode) {
        if ("" != this->_hostname) {
            WiFi.setHostname(this->_hostname.c_str());
        }
        if (true == ap_mode) {
            if (true == this->_config_ap.flag_set) {
                result = WiFi.softAPConfig(this->_config_ap.local_ip, this->_config_ap.gateway, this->_config_ap.subnet);
            }
            if (true == result) {
                result = WiFi.softAP(ssid.c_str(), pass.c_str());
                if (true == result) {
                    this->_mode = MODE_CONNECTION::MODE_CONNECTION_AP;
                }
            }
        } else {
            if (true == this->_config_sta.flag_set) {
                result = WiFi.config(this->_config_sta.local_ip, this->_config_sta.gateway, this->_config_sta.subnet);
            }
            if (true == result) {
                WiFi.begin(ssid.c_str(), pass.c_str());
                int countdown = this->CONNECTION_TIMEOUT_MS;
                while (false == this->is_connected(true)) {
                    delay(this->CONNECTION_TIMEOUT_INTERVAL);
                    countdown -= this->CONNECTION_TIMEOUT_INTERVAL;
                    if (0 >= countdown) {
                        break;
                    }
                }
                result = this->is_connected(true);
                if (true == result) {
                    this->_mode = MODE_CONNECTION::MODE_CONNECTION_STA;
                }
            }
        }
        if (true == result) {
            if (true == save) {
                (void)this->save_information(ssid, pass, ap_mode, auto_default);
            } else {
                (void)this->set_information(ssid, pass, ap_mode, auto_default);
            }
        }
    }
    return result;
}

} // namespace Web
} // namespace MaSiRoProject
