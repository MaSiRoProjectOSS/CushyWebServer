/**
 * @file web_manager_setting.cpp
 * @author Akari (masiro.to.akari@gmail.com)
 * @brief
 * @version 0.0.1
 * @date 2023-03-12
 *
 * @copyright Copyright (c) 2023 / MaSiRo Project.
 *
 */
#include "web_manager_setting.hpp"

#include "../setting_cushy_web_server.hpp"

#if SETTING_WIFI_STORAGE_SPI_FS
#include <SPIFFS.h>
#endif

namespace MaSiRoProject
{
namespace Web
{

WebManagerSetting::WebManagerSetting()
{
    this->_hostname = "";
    (void)this->_default_information_ap();
    (void)this->_default_information_sta();
    this->_open_fs = false;
}

bool WebManagerSetting::_setup()
{
    (void)this->_default_information_ap();
    (void)this->_default_information_sta();

    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    if (0 <= this->_error_count_spi) {
        // SPI FFS doing format if happened error
        if (true == SPIFFS.begin(SETTING_WIFI_STORAGE_SPI_FORMAT)) {
#if SETTING_WIFI_STORAGE_OVERRIDE
            this->_save_information(SETTING_WIFI_AP_SETTING_FILE, SETTING_WIFI_AP_DEFAULT_SSID, SETTING_WIFI_AP_DEFAULT_PASSWORD, SETTING_WIFI_HOSTNAME);
            this->_save_information(SETTING_WIFI_STA_CONNECTED_FILE, SETTING_WIFI_STA_DEFAULT_SSID, SETTING_WIFI_STA_DEFAULT_PASSWORD, SETTING_WIFI_HOSTNAME);
#endif
            if (true == this->_connect_ap) {
                if (true == SPIFFS.exists(SETTING_WIFI_AP_SETTING_FILE)) {
                    // load setting information from file
                    result = this->_load_information(SETTING_WIFI_AP_SETTING_FILE, true);
                }
            }
            if (true == this->_connect_sta) {
                if (true == SPIFFS.exists(SETTING_WIFI_STA_CONNECTED_FILE)) {
                    // load setting information from file
                    result = this->_load_information(SETTING_WIFI_STA_CONNECTED_FILE, false);
                } else {
                    // save setting information because File is not exists
                    result = this->_load_sta_setting(true);
                    while (false == result) {
                        result = this->_load_sta_setting(false);
                    }
                }
            }

            SPIFFS.end();
        } else {
            this->_error_count_spi--;
        }
        this->_open_fs = result;
    } else {
        // override setting information
        this->_open_fs = false;
        //result         = this->save_information(this->_ssid, this->_pass, this->_mode_ap, this->_auto_default_setting);
    }
#endif
    return result;
}

bool WebManagerSetting::save_information_sta(std::string ssid, std::string pass, std::string hostname)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    if (true == this->_open_fs) {
        if (true == SPIFFS.begin()) {
            bool force_write = false;
            if (true == SPIFFS.exists(SETTING_WIFI_STA_CONNECTED_FILE)) {
                this->_load_information(SETTING_WIFI_STA_CONNECTED_FILE, false);
            } else {
                force_write = true;
            }
            if ((true == force_write) //
                || (this->_sta_ssid != ssid) || (this->_sta_pass != pass)) {
                force_write = false;

                result = _save_information(SETTING_WIFI_STA_CONNECTED_FILE, ssid, pass, hostname);
            }
            SPIFFS.end();
            if (true == force_write) {
                ssid = this->_sta_ssid;
                pass = this->_sta_pass;
            }
        }
    } else {
        if (0 > this->_error_count_spi) {
            result = true;
        }
    }
#else
    result = true;
#endif
    (void)this->_set_information_sta(ssid, pass, this->_hostname);
    return result;
}
bool WebManagerSetting::save_information_ap(std::string ssid, std::string pass, std::string hostname)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    if (true == this->_open_fs) {
        if (true == SPIFFS.begin()) {
            bool force_write = false;
            if (true == SPIFFS.exists(SETTING_WIFI_AP_SETTING_FILE)) {
                this->_load_information(SETTING_WIFI_AP_SETTING_FILE, true);
            } else {
                force_write = true;
            }
            if ((true == force_write)                                   //
                || (this->_ap_ssid != ssid) || (this->_ap_pass != pass) //
            ) {
                force_write = false;
                result      = _save_information(SETTING_WIFI_AP_SETTING_FILE, ssid, pass, hostname);
            }
            SPIFFS.end();
            if (true == force_write) {
                ssid = this->_ap_ssid;
                pass = this->_ap_pass;
            }
        }
    } else {
        if (0 > this->_error_count_spi) {
            result = true;
        }
    }
#else
    result = true;
#endif
    (void)this->_set_information_ap(ssid, pass, hostname);
    return result;
}
bool WebManagerSetting::load_information_ap()
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    if (true == this->_open_fs) {
        if (SPIFFS.begin()) {
            if (true == SPIFFS.exists(SETTING_WIFI_AP_SETTING_FILE)) {
                result = this->_load_information(SETTING_WIFI_AP_SETTING_FILE, true);
            }
            SPIFFS.end();
        }
    } else {
        if (0 > this->_error_count_spi) {
            result = this->_default_information_ap();
        }
    }
#else
    result = this->_default_information_ap();
#endif
    return result;
}

bool WebManagerSetting::load_sta_settings(bool clear)
{
    bool result = false;
    //////////////////
#if SETTING_WIFI_STORAGE_SPI_FS
    char buffer[255];
    if (SPIFFS.begin()) {
        result = this->_load_sta_setting(clear);
        SPIFFS.end();
    }
#endif
    //////////////////
    if (false == result) {
        result = this->_default_information_sta();
    }

    return result;
}

void WebManagerSetting::set_information_ap(std::string ssid, std::string pass, std::string hostname)
{
    this->_set_information_ap(ssid, pass, hostname);
}
void WebManagerSetting::set_information_sta(std::string ssid, std::string pass, std::string hostname)
{
    this->_set_information_sta(ssid, pass, hostname);
}

void WebManagerSetting::set_hostname(std::string hostname)
{
    this->_hostname = (std::string)hostname;
}

////////////////////////////////////////////////////
// private function
////////////////////////////////////////////////////
void WebManagerSetting::_set_information_ap(std::string ssid, std::string pass, std::string hostname)
{
    this->_ap_ssid  = (std::string)ssid;
    this->_ap_pass  = (std::string)pass;
    this->_hostname = (std::string)hostname;

    log_v("Set information : MODE[A P] SSID[%s]", this->_sta_ssid.c_str());
}
void WebManagerSetting::_set_information_sta(std::string ssid, std::string pass, std::string hostname)
{
    this->_sta_ssid = (std::string)ssid;
    this->_sta_pass = (std::string)pass;
    this->_hostname = (std::string)hostname;

    log_v("Set information : MODE[STA] SSID[%s]", this->_ap_ssid.c_str());
}

bool WebManagerSetting::_load_information(std::string file, bool mode_ap)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    log_d("load file: %s", file.c_str());
    if (true == SPIFFS.exists(file.c_str())) {
        File dataFile = SPIFFS.open(file.c_str(), FILE_READ);
        if (!dataFile) {
            result = false;
        } else {
            int type = 0;
            result   = true;
            // int totalBytes = dataFile.size();
            while (dataFile.available()) {
                String word = dataFile.readStringUntil('\n');
                switch (type) {
                    case 0:
                        if (0 < word.length()) {
                            if (true == mode_ap) {
                                this->_ap_ssid = word.c_str();
                            } else {
                                this->_sta_ssid = word.c_str();
                            }
                        }
                        break;
                    case 1:
                        if (0 < word.length()) {
                            if (true == mode_ap) {
                                this->_ap_pass = word.c_str();
                            } else {
                                this->_sta_pass = word.c_str();
                            }
                        }
                        break;
                    case 2:
                        if (0 < word.length()) {
                            if (true == mode_ap) {
                                this->_hostname = "";
                            } else {
                                this->_hostname = word.c_str();
                            }
                        } else {
                            this->_hostname = "";
                        }
                        break;

                    default:
                        break;
                }
                type++;
                if (2 < type) {
                    break;
                }
            }
            dataFile.close();
        }
    }
#else
    this->_default_information();
    result = true;
#endif
    log_v("Load information : MODE[%s] SSID[%s] HOSTNAME[%s]",
          (true == mode_ap) ? "AP" : "STA",
          (true == mode_ap) ? this->_ap_ssid.c_str() : this->_sta_ssid.c_str(),
          (true == mode_ap) ? "" : this->_hostname.c_str());

    return result;
}
bool WebManagerSetting::_save_information(std::string file, std::string ssid, std::string pass, std::string hostname)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    log_d("Save information");
    File dataFile = SPIFFS.open(file.c_str(), FILE_WRITE);
    dataFile.printf("%s\n", ssid.c_str());
    dataFile.printf("%s\n", pass.c_str());
    dataFile.printf("%s\n", hostname.c_str());
    dataFile.close();
    result = true;
#else
    result = true;
#endif
    return result;
}
bool WebManagerSetting::_default_information_sta()
{
    (void)this->_set_information_sta(SETTING_WIFI_STA_DEFAULT_SSID, SETTING_WIFI_STA_DEFAULT_PASSWORD, SETTING_WIFI_HOSTNAME);
    return true;
}
bool WebManagerSetting::_default_information_ap()
{
    (void)this->_set_information_ap(SETTING_WIFI_AP_DEFAULT_SSID, SETTING_WIFI_AP_DEFAULT_PASSWORD, SETTING_WIFI_HOSTNAME);
    return true;
}
bool WebManagerSetting::_load_sta_setting(bool clear)
{
    bool result = false;
    if (true == clear) {
        this->_sta_explored_index = 0;
    }
    //////////////////
    int count_up = this->_sta_explored_index;
#if SETTING_WIFI_STORAGE_SPI_FS
    char buffer[255];
    for (int i = this->_sta_explored_index; i < SETTING_WIFI_STA_FILE_MAX; i++) {
        sprintf(buffer, SETTING_WIFI_STA_FILE_PATTERN, i);
        if (true == SPIFFS.exists(buffer)) {
            result = this->_load_information(buffer, false);
            break;
        }
        count_up++;
    }
#endif
    if (SETTING_WIFI_STA_FILE_MAX >= this->_sta_explored_index) {
        this->_sta_explored_index = count_up + 1;
    }
    //////////////////
#if SETTING_WIFI_STA_LOOP_FILE
    if (false == result) {
        result = this->_load_sta_setting(true);
    }
#endif
    //////////////////
    if (false == result) {
        result = this->_default_information_sta();
    }

    return result;
}

} // namespace Web
} // namespace MaSiRoProject
