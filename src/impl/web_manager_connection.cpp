/**
 * @file web_manager_connection.cpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.0.1
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
bool WebManagerConnection::is_enable_ap()
{
    return this->_connect_ap;
}
IPAddress WebManagerConnection::get_ip_address_ap()
{
    return WiFi.softAPIP();
}
const char *WebManagerConnection::get_ssid_ap()
{
    return this->_ap_ssid.c_str();
}
bool WebManagerConnection::is_enable_sta()
{
    return this->_connect_sta;
}
IPAddress WebManagerConnection::get_ip_address_sta()
{
    return WiFi.localIP();
}
const char *WebManagerConnection::get_ssid_sta()
{
    return this->_sta_ssid.c_str();
}

bool WebManagerConnection::is_connected_sta(bool immediate)
{
    static unsigned long next_time = 0;
    static bool result             = false;
    if ((true == immediate) || (next_time < millis())) {
        next_time        = millis() + this->CONNECTION_CHECK_INTERVAL;
        wifi_mode_t mode = WiFi.getMode();
        wl_status_t status;
        switch (mode) {
            case wifi_mode_t::WIFI_MODE_STA:
            case wifi_mode_t::WIFI_MODE_APSTA:
                status = WiFi.status();
                switch (status) {
                    case wl_status_t::WL_CONNECTED:
                        result = true;
                        break;
                    case wl_status_t::WL_SCAN_COMPLETED:
                    case wl_status_t::WL_IDLE_STATUS:
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
                result = false;
                break;
        }
    }
    return result;
}
bool WebManagerConnection::is_connected_ap(bool immediate)
{
    static unsigned long next_time = 0;
    static bool result             = false;
    if ((true == immediate) || (next_time < millis())) {
        next_time        = millis() + this->CONNECTION_CHECK_INTERVAL;
        wifi_mode_t mode = WiFi.getMode();
        switch (mode) {
            case wifi_mode_t::WIFI_MODE_AP:
            case wifi_mode_t::WIFI_MODE_APSTA:
                result = true;
                break;
            case wifi_mode_t::WIFI_MODE_STA:
            default:
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
    this->set_hostname(SETTING_WIFI_HOSTNAME);
    return this->_setup();
}

bool WebManagerConnection::reconnect_ap(bool save)
{
    return this->reconnect_ap(this->_ap_ssid, this->_ap_pass, save);
}
bool WebManagerConnection::reconnect_sta(bool save)
{
    return this->reconnect_sta(this->_sta_ssid, this->_sta_pass, -1, save);
}
bool WebManagerConnection::reconnect_ap(std::string ssid, std::string pass, bool save)
{
    if ("" == ssid) {
        ssid = this->_ap_ssid;
    }
    if ("" == pass) {
        pass = this->_ap_pass;
    }
    bool result = this->_reconnect_ap(ssid, pass, save);
    if (false == result) {
        if ((this->_ap_ssid != ssid) || (this->_ap_pass != pass)) {
            result = this->_reconnect_ap(this->_ap_ssid, this->_ap_pass, false);
        }
    }
    if (true == result) {
        this->_connect_ap = result;
    }
    return result;
}
bool WebManagerConnection::reconnect_sta(std::string ssid, std::string pass, int num, bool save)
{
    if ("" == ssid) {
        ssid = this->_sta_ssid;
    }
    if ("" == pass) {
        pass = this->_sta_pass;
    }
    bool result = this->_reconnect_sta(ssid, pass, save, num);
    if (false == result) {
        if ((this->_sta_ssid != ssid) || (this->_sta_pass != pass)) {
            result = this->_reconnect_sta(this->_sta_ssid, this->_sta_pass, num, false);
        }
    }
    if (true == result) {
        this->_sta_explored_index = 0;
        this->_connect_sta        = result;
    }
    return result;
}

bool WebManagerConnection::disconnect_ap()
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
        while (true == this->is_connected_ap(true)) {
            delay(this->CONNECTION_TIMEOUT_INTERVAL);
            countdown -= this->CONNECTION_TIMEOUT_INTERVAL;
            if (0 >= countdown) {
                break;
            }
        }
    }
    result = !this->is_connected_ap(true);
    return result;
}
bool WebManagerConnection::disconnect_sta()
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
        while (true == this->is_connected_sta(true)) {
            delay(this->CONNECTION_TIMEOUT_INTERVAL);
            countdown -= this->CONNECTION_TIMEOUT_INTERVAL;
            if (0 >= countdown) {
                break;
            }
        }
    }
    result = !this->is_connected_sta(true);
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

bool WebManagerConnection::_reconnect_ap(std::string ssid, std::string pass, bool save)
{
    bool result = true;
    (void)this->disconnect_ap();
    if ("" != this->_hostname) {
        WiFi.setHostname(this->_hostname.c_str());
    }
    if (true == this->_config_ap.flag_set) {
        result = WiFi.softAPConfig(this->_config_ap.local_ip, this->_config_ap.gateway, this->_config_ap.subnet);
    }
    if (true == result) {
        result = WiFi.softAP(ssid.c_str(), pass.c_str());
    }
    if (true == result) {
        if (true == save) {
            (void)this->save_information_ap(ssid, pass, this->_hostname);
        } else {
            (void)this->set_information_ap(ssid, pass);
        }
    }
    return result;
}

bool WebManagerConnection::_reconnect_sta(std::string ssid, std::string pass, int num, bool save)
{
    bool result = true;
    (void)this->disconnect_sta();
    if ("" != this->_hostname) {
        WiFi.setHostname(this->_hostname.c_str());
    }
    if (true == this->_config_sta.flag_set) {
        result = WiFi.config(this->_config_sta.local_ip, this->_config_sta.gateway, this->_config_sta.subnet);
    }
    if (true == result) {
        WiFi.begin(ssid.c_str(), pass.c_str());
        int countdown = this->CONNECTION_TIMEOUT_MS;
        while (false == this->is_connected_sta(true)) {
            delay(this->CONNECTION_TIMEOUT_INTERVAL);
            countdown -= this->CONNECTION_TIMEOUT_INTERVAL;
            if (0 >= countdown) {
                break;
            }
        }
        result = this->is_connected_sta(true);
    }
    if (true == result) {
        if (true == save) {
            (void)this->save_information_sta(ssid, pass, this->_hostname, num);
        } else {
            (void)this->set_information_sta(ssid, pass);
        }
    }
    return result;
}
} // namespace Web
} // namespace MaSiRoProject
