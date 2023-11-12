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

#include <aes/esp_aes.h>
#include <base64.h>

namespace MaSiRoProject
{
namespace Web
{
#define INPUT_BUFFER_LIMIT (128 + 1)
#define N_BLOCK            (32)

const uint8_t ia[16]      = { 0x2F, 0x7E, 0x25, 0x66, 0x38, 0xB4, 0xA1, 0x47, 0x43, 0x90, 0xF1, 0x83, 0x54, 0xF1, 0x33, 0x8B };
unsigned char ik_shift    = 0xA0;
unsigned char ik[N_BLOCK] = { 0x0A };
int ik_len                = 0;

inline unsigned char lookup_b64(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    if (c >= 'a' && c <= 'z')
        return c - 71;
    if (c >= '0' && c <= '9')
        return c + 4;
    if (c == '+')
        return 62;
    if (c == '/')
        return 63;
    return -1;
}

inline int decode_base64(uint8_t *output, const char *input, int inputLen)
{
    int i      = 0;
    int j      = 0;
    int decLen = 0;
    unsigned char a4[4];

    while (inputLen--) {
        if (*input == '=') {
            break;
        }

        a4[i++] = *(input++);
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                a4[i] = lookup_b64(a4[i]);
            }
            output[decLen++] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
            output[decLen++] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
            output[decLen++] = ((a4[2] & 0x3) << 6) + a4[3];

            i = 0;
        }
    }
    if (0 != i) {
        for (j = i; j < 4; j++) {
            a4[j] = '\0';
        }
        for (j = 0; j < 4; j++) {
            a4[j] = lookup_b64(a4[j]);
        }
        output[decLen++] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
        output[decLen++] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
        output[decLen++] = ((a4[2] & 0x3) << 6) + a4[3];
    }
    output[decLen] = '\0';
    return decLen;
}

int cbc_base64(const uint8_t key[32], const uint8_t iv[16], const char *plaintext, char *encrypted)
{
    char cip[2 * INPUT_BUFFER_LIMIT] = { 0 };
    esp_aes_context context;
    uint8_t iv_buff[16];
    uint8_t *output;
    memcpy(iv_buff, iv, sizeof(iv_buff));

    esp_aes_init(&context);
    esp_aes_setkey(&context, key, 256);
    int len = ((strlen(plaintext) / 16) + 1) * 16;
    esp_aes_crypt_cbc(&context, ESP_AES_ENCRYPT, len, iv_buff, (const unsigned char *)plaintext, (unsigned char *)cip);
    esp_aes_free(&context);

    String base = base64::encode((const uint8_t *)cip, len);
    sprintf(encrypted, "%s", base.c_str());
    return len;
}

int cbc_base64_to_text(const uint8_t key[32], const uint8_t iv[16], const char *encrypted, char *plaintext)
{
    esp_aes_context context;
    uint8_t iv_buff[16];
    uint8_t b_text[2 * INPUT_BUFFER_LIMIT];
    memcpy(iv_buff, iv, sizeof(iv_buff));
    decode_base64(b_text, encrypted, strlen(encrypted));

    esp_aes_init(&context);
    esp_aes_setkey(&context, key, 256);
    int len = ((strlen(encrypted) / 16) + 1) * 16;
    esp_aes_crypt_cbc(&context, ESP_AES_DECRYPT, len, iv_buff, b_text, (uint8_t *)plaintext);
    esp_aes_free(&context);

    return len;
}

//////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////
WebManagerSetting::WebManagerSetting() : _error_count_spi(ERROR_COUNT_SPI_MAX), _open_fs(false)
{
    uint8_t base_mac[6];
    char base_mac_chr[18] = { 0 };
    char buffer[255];
    esp_read_mac(base_mac, ESP_MAC_WIFI_STA);
    sprintf(base_mac_chr, "%02X%02X%02X%02X%02X%02X", base_mac[0], base_mac[1], base_mac[2], base_mac[3], base_mac[4], base_mac[5]);
    ik_len = sprintf(buffer, "%s-sta%sap%s", base_mac_chr, SETTING_WIFI_STA_DEFAULT_SSID, SETTING_WIFI_AP_DEFAULT_SSID);
    for (int i = 0; (i < ik_len) & (i < N_BLOCK); i++) {
        ik[i] = (unsigned char)((buffer[i] + ik_shift) & 0xFF);
    }
    if (true == SPIFFS.begin()) {
        this->_init_sta_setting(SPIFFS);
        SPIFFS.end();
    }
    (void)this->set_hostname(SETTING_WIFI_HOSTNAME);
    (void)this->set_ap_information(SETTING_WIFI_AP_DEFAULT_SSID, SETTING_WIFI_AP_DEFAULT_PASSWORD);
    (void)this->set_sta_information(SETTING_WIFI_STA_DEFAULT_SSID, SETTING_WIFI_STA_DEFAULT_PASSWORD);
}

//////////////////////////////////////////////////////////////
// Public function
//////////////////////////////////////////////////////////////
// AP mode
bool WebManagerSetting::set_ap_enable(bool flag)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    char buffer[255];
    if (true == this->_open_fs) {
        if (true == SPIFFS.begin()) {
            this->_save_settings_wifi(SPIFFS, flag, this->_enable_sta);
            SPIFFS.end();
        }
    } else {
        if (0 > this->_error_count_spi) {
            result = true;
        }
    }
#else
    result = true;
#endif
    return result;
}
bool WebManagerSetting::save_ap_setting(bool enable, std::string ssid, std::string pass)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    char buffer[255];
    if (true == this->_open_fs) {
        if (true == SPIFFS.begin()) {
            this->_save_settings_wifi(SPIFFS, enable, this->_enable_sta);
            if (true == enable) {
                result = this->_save_information(SPIFFS, SETTING_WIFI_AP_SETTING_FILE, ssid, pass, this->_hostname);
            }
            SPIFFS.end();
        }
    } else {
        if (0 > this->_error_count_spi) {
            result = true;
        }
    }
#else
    result = true;
#endif
    return result;
}
bool WebManagerSetting::save_ap_information(std::string ssid, std::string pass, std::string hostname)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    if (true == this->_open_fs) {
        if (true == SPIFFS.begin()) {
            bool force_write = false;
            if (true == SPIFFS.exists(SETTING_WIFI_AP_SETTING_FILE)) {
                this->_load_information(SPIFFS, SETTING_WIFI_AP_SETTING_FILE, true);
            } else {
                force_write = true;
            }
            if ((true == force_write)                                   //
                || (this->_ap_ssid != ssid) || (this->_ap_pass != pass) //
            ) {
                force_write = false;
                result      = _save_information(SPIFFS, SETTING_WIFI_AP_SETTING_FILE, ssid, pass, hostname);
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
    ssid   = SETTING_WIFI_AP_DEFAULT_SSID;
    pass   = SETTING_WIFI_AP_DEFAULT_PASSWORD;
    result = true;
#endif
    (void)this->set_hostname(hostname);
    (void)this->set_ap_information(ssid, pass);

    return result;
}
bool WebManagerSetting::load_ap_settings()
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    if (true == this->_open_fs) {
        if (SPIFFS.begin()) {
            if (true == SPIFFS.exists(SETTING_WIFI_AP_SETTING_FILE)) {
                result = this->_load_information(SPIFFS, SETTING_WIFI_AP_SETTING_FILE, true);
            }
            SPIFFS.end();
        }
    } else {
        if (0 > this->_error_count_spi) {
            (void)this->set_ap_information(SETTING_WIFI_AP_DEFAULT_SSID, SETTING_WIFI_AP_DEFAULT_PASSWORD);
            result = true;
        }
    }
#else
    (void)this->set_ap_information(SETTING_WIFI_AP_DEFAULT_SSID, SETTING_WIFI_AP_DEFAULT_PASSWORD, SETTING_WIFI_HOSTNAME);
    result            = true;
#endif
    return result;
}

// STA mode
bool WebManagerSetting::set_sta_enable(bool flag)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    char buffer[255];
    if (true == this->_open_fs) {
        if (true == SPIFFS.begin()) {
            this->_save_settings_wifi(SPIFFS, this->_enable_ap, flag);
            SPIFFS.end();
        }
    } else {
        if (0 > this->_error_count_spi) {
            result = true;
        }
    }
#else
    result            = true;
#endif
    return result;
}
bool WebManagerSetting::save_sta_setting(bool enable, std::string ssid, std::string pass, std::string hostname, int num)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    char buffer[255];
    if (true == this->_open_fs) {
        if (true == SPIFFS.begin()) {
            this->_save_settings_wifi(SPIFFS, this->_enable_ap, enable);
            if (true == enable) {
                if (0 <= num) {
                    if (num < SETTING_WIFI_STA_FILE_MAX) {
                        sprintf(buffer, SETTING_WIFI_STA_FILE_PATTERN, num);
                        result = this->_save_information(SPIFFS, buffer, ssid, pass, hostname);
                    }
                }
            }
            SPIFFS.end();
        }
    } else {
        if (0 > this->_error_count_spi) {
            result = true;
        }
    }
#else
    result            = true;
#endif
    return result;
}
bool WebManagerSetting::save_sta_information(std::string ssid, std::string pass, std::string hostname, int num)
{
    bool result = false;
    char buffer[255];
#if SETTING_WIFI_STORAGE_SPI_FS
    if (true == this->_open_fs) {
        if (true == SPIFFS.begin()) {
            bool force_write = false;
            if (true == SPIFFS.exists(SETTING_WIFI_STA_CONNECTED_FILE)) {
                this->_load_information(SPIFFS, SETTING_WIFI_STA_CONNECTED_FILE, false);
            } else {
                force_write = true;
            }
            if ((true == force_write) //
                || (this->_sta_ssid != ssid) || (this->_sta_pass != pass)) {
                force_write = false;

                result = this->_save_information(SPIFFS, SETTING_WIFI_STA_CONNECTED_FILE, ssid, pass, hostname);
                if (0 <= num) {
                    if (num < SETTING_WIFI_STA_FILE_MAX) {
                        sprintf(buffer, SETTING_WIFI_STA_FILE_PATTERN, num);
                        this->_save_information(SPIFFS, buffer, ssid, pass, hostname);
                    }
                }
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
    ssid              = SETTING_WIFI_STA_DEFAULT_SSID;
    pass              = SETTING_WIFI_STA_DEFAULT_PASSWORD;
    result            = true;
#endif
    (void)this->set_sta_information(ssid, pass);
    return result;
}
bool WebManagerSetting::load_sta_settings(bool clear)
{
    bool result = false;
    //////////////////
#if SETTING_WIFI_STORAGE_SPI_FS
    char buffer[255];
    if (SPIFFS.begin()) {
        result = this->_load_sta_setting(SPIFFS, clear);
        SPIFFS.end();
    }
#endif
    //////////////////
    if (false == result) {
        (void)this->set_sta_information(SETTING_WIFI_STA_DEFAULT_SSID, SETTING_WIFI_STA_DEFAULT_PASSWORD);
        result = true;
    }

    return result;
}

//////////////////////////////////////////////////////////////
// Protected function
//////////////////////////////////////////////////////////////
bool WebManagerSetting::_setup()
{
    (void)this->set_ap_information(SETTING_WIFI_AP_DEFAULT_SSID, SETTING_WIFI_AP_DEFAULT_PASSWORD);
    (void)this->set_sta_information(SETTING_WIFI_STA_DEFAULT_SSID, SETTING_WIFI_STA_DEFAULT_PASSWORD);

    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    if (0 <= this->_error_count_spi) {
        // SPI FFS doing format if happened error
        if (true == SPIFFS.begin(SETTING_WIFI_STORAGE_SPI_FORMAT)) {
#if SETTING_WIFI_STORAGE_OVERRIDE
            this->_save_settings_wifi(SPIFFS, SETTING_WIFI_AP_DEFAULT_ENABLE, SETTING_WIFI_STA_DEFAULT_ENABLE);
            this->_save_information(SPIFFS, SETTING_WIFI_AP_SETTING_FILE, SETTING_WIFI_AP_DEFAULT_SSID, SETTING_WIFI_AP_DEFAULT_PASSWORD, SETTING_WIFI_HOSTNAME);
            this->_save_information(SPIFFS, SETTING_WIFI_STA_CONNECTED_FILE, SETTING_WIFI_STA_DEFAULT_SSID, SETTING_WIFI_STA_DEFAULT_PASSWORD, SETTING_WIFI_HOSTNAME);
#endif

            // WiFi Settings
            if (false == SPIFFS.exists(SETTING_WIFI_SETTING_FILE)) {
                this->_save_settings_wifi(SPIFFS, SETTING_WIFI_AP_DEFAULT_ENABLE, SETTING_WIFI_STA_DEFAULT_ENABLE);
            } else {
                this->_load_settings_wifi(SPIFFS);
            }

            // AP Mode settings
            if (false == SPIFFS.exists(SETTING_WIFI_AP_SETTING_FILE)) {
                this->_save_information(SPIFFS, SETTING_WIFI_AP_SETTING_FILE, SETTING_WIFI_AP_DEFAULT_SSID, SETTING_WIFI_AP_DEFAULT_PASSWORD, SETTING_WIFI_HOSTNAME);
            }
            if (true == SPIFFS.exists(SETTING_WIFI_AP_SETTING_FILE)) {
                result = this->_load_information(SPIFFS, SETTING_WIFI_AP_SETTING_FILE, true);
            }

            // STA Mode settings
            if (false == SPIFFS.exists(SETTING_WIFI_STA_CONNECTED_FILE)) {
                this->_save_information(SPIFFS, SETTING_WIFI_STA_CONNECTED_FILE, SETTING_WIFI_STA_DEFAULT_SSID, SETTING_WIFI_STA_DEFAULT_PASSWORD, SETTING_WIFI_HOSTNAME);
            }
            if (true == SPIFFS.exists(SETTING_WIFI_STA_CONNECTED_FILE)) {
                result = this->_load_information(SPIFFS, SETTING_WIFI_STA_CONNECTED_FILE, false);
            } else {
                result = this->_load_sta_setting(SPIFFS, true);
                while (false == result) {
                    result = this->_load_sta_setting(SPIFFS, false);
                }
            }

            SPIFFS.end();
            this->_error_count_spi = this->ERROR_COUNT_SPI_MAX;
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

    log_d("%s", ((true == result) ? "Setup was successful." : "Setup failed."));
    return result;
}

//////////////////////////////////////////////////////////////
// Private function
//////////////////////////////////////////////////////////////

// ---------------------------------------------------------//
// WiFi settings
// ---------------------------------------------------------//
bool WebManagerSetting::_load_settings_wifi(fs::FS &fs)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    if (true == fs.exists(SETTING_WIFI_SETTING_FILE)) {
        File dataFile = fs.open(SETTING_WIFI_SETTING_FILE, FILE_READ);
        if (!dataFile) {
            result = false;
        } else {
            bool flag_break = false;
            int line        = 1;
            result          = true;

            while (dataFile.available()) {
                String word = dataFile.readStringUntil('\n');
                word.replace("\r", "");
                word.replace("\n", "");
                switch (line) {
                    case 1:
                        if (0 < word.length()) {
                            this->_enable_ap = (word.equals("1")) ? true : false;
                        }
                        break;
                    case 2:
                        if (0 < word.length()) {
                            this->_enable_sta = (word.equals("1")) ? true : false;
                        }
                        break;

                    default:
                        flag_break = true;
                        break;
                }
                line++;
                if (true == flag_break) {
                    break;
                }
            }
            dataFile.close();
        }
    }
#else
    this->_enable_ap  = SETTING_WIFI_AP_DEFAULT_ENABLE;
    this->_enable_sta = SETTING_WIFI_STA_DEFAULT_ENABLE;
    result            = true;
#endif
    log_d("Load settings: %s : AP[%s] STA[%s]", //
          SETTING_WIFI_SETTING_FILE,
          (true == this->_enable_ap) ? "True" : "False",
          (true == this->_enable_sta) ? "True" : "False");
    return result;
}
bool WebManagerSetting::_save_settings_wifi(fs::FS &fs, bool ap_mode, bool sta_mode)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    log_d("Save settings: %s", SETTING_WIFI_SETTING_FILE);
    File dataFile = fs.open(SETTING_WIFI_SETTING_FILE, FILE_WRITE);
    dataFile.printf("%d\n", (ap_mode ? 1 : 0));
    dataFile.printf("%d\n", (sta_mode ? 1 : 0));
    dataFile.close();
    result = true;
#else
    result            = true;
#endif
    this->_enable_ap  = ap_mode;
    this->_enable_sta = sta_mode;
    return result;
}

//////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////

void WebManagerSetting::set_ap_information(std::string ssid, std::string pass)
{
    this->_ap_ssid = (std::string)ssid;
    this->_ap_pass = (std::string)pass;
    log_v("Set information : MODE[A P] SSID[%s]", this->_ap_ssid.c_str());
}
void WebManagerSetting::set_sta_information(std::string ssid, std::string pass)
{
    this->_sta_ssid = (std::string)ssid;
    this->_sta_pass = (std::string)pass;
    log_v("Set information : MODE[STA] SSID[%s]", this->_sta_ssid.c_str());
}

void WebManagerSetting::set_hostname(std::string hostname)
{
    this->_hostname = (std::string)hostname;
}
int WebManagerSetting::get_sta_list_selected()
{
    if (true == _enable_sta) {
        return this->_sta_list_selected;
    } else {
        return -1;
    }
}
std::string WebManagerSetting::get_sta_list_ssid(int index)
{
    if (0 <= index) {
        if (index < SETTING_WIFI_STA_FILE_MAX) {
            return this->_sta_list_ssid[index];
        }
    }
    return "";
}
std::string WebManagerSetting::get_sta_list_hostname(int index)
{
    if (0 <= index) {
        if (index < SETTING_WIFI_STA_FILE_MAX) {
            return this->_sta_list_hostname[index];
        }
    }
    return "";
}

////////////////////////////////////////////////////
// private function
////////////////////////////////////////////////////

bool WebManagerSetting::_load_information(fs::FS &fs, std::string file, bool mode_ap)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    char cip[2 * INPUT_BUFFER_LIMIT] = { 0 };
    char buf[2 * INPUT_BUFFER_LIMIT] = { 0 };
    uint16_t len;
    if (true == SPIFFS.exists(file.c_str())) {
        File dataFile = SPIFFS.open(file.c_str(), FILE_READ);
        if (!dataFile) {
            result = false;
        } else {
            bool flag_break = false;
            int line        = 1;
            result          = true;
            // int totalBytes = dataFile.size();
            while (dataFile.available()) {
                String word = dataFile.readStringUntil('\n');
                word.replace("\r", "");
                word.replace("\n", "");
                len = cbc_base64_to_text(ik, ia, word.c_str(), cip);
                sprintf(buf, "%s", cip);
                switch (line) {
                    case 1:
                        if (0 < word.length()) {
                            if (true == mode_ap) {
                                this->_ap_ssid = buf;
                            } else {
                                this->_sta_ssid = buf;
                            }
                        }
                        break;
                    case 2:
                        if (0 < word.length()) {
                            if (true == mode_ap) {
                                this->_ap_pass = buf;
                            } else {
                                this->_sta_pass = buf;
                            }
                        }
                        break;
                    case 3:
                        if (0 < word.length()) {
                            if (false == mode_ap) {
                                this->_hostname = buf;
                            }
                        } else {
                            this->_hostname = "";
                        }
                        break;

                    default:
                        flag_break = true;
                        break;
                }
                line++;
                if (true == flag_break) {
                    break;
                }
            }
            dataFile.close();
        }
    }
#else
    (void)this->set_ap_information(SETTING_WIFI_AP_DEFAULT_SSID, SETTING_WIFI_AP_DEFAULT_PASSWORD, SETTING_WIFI_HOSTNAME);
    (void)this->set_sta_information(SETTING_WIFI_STA_DEFAULT_SSID, SETTING_WIFI_STA_DEFAULT_PASSWORD, SETTING_WIFI_HOSTNAME);
    result = true;
#endif
    log_d("Load information : MODE[%s] SSID[%s] HOSTNAME[%s] filename[%s]",
          (true == mode_ap) ? "A P" : "STA",
          (true == mode_ap) ? this->_ap_ssid.c_str() : this->_sta_ssid.c_str(),
          (true == mode_ap) ? "" : this->_hostname.c_str(),
          file.c_str());
    return result;
}
bool WebManagerSetting::_save_information(fs::FS &fs, std::string file, std::string ssid, std::string pass, std::string hostname)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    log_d("Save information: %s", file.c_str());
    char cip[2 * INPUT_BUFFER_LIMIT] = { 0 };
    uint16_t len;
    File dataFile = fs.open(file.c_str(), FILE_WRITE);
    len           = cbc_base64(ik, ia, ssid.c_str(), cip);
    dataFile.printf("%s\n", (char *)cip);
    len = cbc_base64(ik, ia, pass.c_str(), cip);
    dataFile.printf("%s\n", (char *)cip);
    len = cbc_base64(ik, ia, hostname.c_str(), cip);
    dataFile.printf("%s\n", (char *)cip);
    dataFile.close();
    result = true;
#else
    result = true;
#endif
    return result;
}
void WebManagerSetting::_init_sta_setting(fs::FS &fs)
{
#if SETTING_WIFI_STORAGE_SPI_FS
    char buffer[255];
    for (int i = 0; i < SETTING_WIFI_STA_FILE_MAX; i++) {
        sprintf(buffer, SETTING_WIFI_STA_FILE_PATTERN, i);
        if (true == fs.exists(buffer)) {
            if (true == this->_load_information(fs, buffer, false)) {
                this->_sta_list_ssid[i]     = this->_sta_ssid;
                this->_sta_list_hostname[i] = this->_hostname;
            }
        }
    }
#endif
}
bool WebManagerSetting::_load_sta_setting(fs::FS &fs, bool clear)
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
        count_up = i + 1;
        sprintf(buffer, SETTING_WIFI_STA_FILE_PATTERN, i);
        if (true == fs.exists(buffer)) {
            result = this->_load_information(fs, buffer, false);
            if (true == result) {
                this->_sta_list_ssid[i]     = this->_sta_ssid;
                this->_sta_list_hostname[i] = this->_hostname;
                this->_sta_list_selected    = i;
            }
            break;
        }
    }
#endif
    this->_sta_explored_index = count_up;
    if (false == result) {
        (void)this->set_sta_information(SETTING_WIFI_STA_DEFAULT_SSID, SETTING_WIFI_STA_DEFAULT_PASSWORD);
        this->_sta_explored_index = 0;
        result                    = true;
    }

    return result;
}

} // namespace Web
} // namespace MaSiRoProject
