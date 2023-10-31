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

int decode_base64(uint8_t *output, const char *input, int inputLen)
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

WebManagerSetting::WebManagerSetting()
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
                if (false == SPIFFS.exists(SETTING_WIFI_AP_SETTING_FILE)) {
                    this->_save_information(SETTING_WIFI_AP_SETTING_FILE, SETTING_WIFI_AP_DEFAULT_SSID, SETTING_WIFI_AP_DEFAULT_PASSWORD, SETTING_WIFI_HOSTNAME);
                }
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
    log_d("%s", ((true == result) ? "Setup was successful." : "Setup failed."));

    return result;
}

bool WebManagerSetting::save_information_sta(std::string ssid, std::string pass, std::string hostname, int num)
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

                result = this->_save_information(SETTING_WIFI_STA_CONNECTED_FILE, ssid, pass, hostname);
                if (0 <= num) {
                    if (num < SETTING_WIFI_STA_FILE_MAX) {
                        char buffer[255];
                        sprintf(buffer, SETTING_WIFI_STA_FILE_PATTERN, num);
                        this->_save_information(buffer, ssid, pass, hostname);
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

    log_d("Set information : MODE[A P] SSID[%s]", this->_ap_ssid.c_str());
}
void WebManagerSetting::_set_information_sta(std::string ssid, std::string pass, std::string hostname)
{
    this->_sta_ssid = (std::string)ssid;
    this->_sta_pass = (std::string)pass;
    this->_hostname = (std::string)hostname;

    log_d("Set information : MODE[STA] SSID[%s]", this->_sta_ssid.c_str());
}

bool WebManagerSetting::_load_information(std::string file, bool mode_ap)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    char cip[2 * INPUT_BUFFER_LIMIT] = { 0 };
    char buf[2 * INPUT_BUFFER_LIMIT] = { 0 };
    uint16_t len;
    log_v("load file: %s", file.c_str());
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
                word.replace("\r", "");
                word.replace("\n", "");
                len = cbc_base64_to_text(ik, ia, word.c_str(), cip);
                sprintf(buf, "%s", cip);
                switch (type) {
                    case 0:
                        if (0 < word.length()) {
                            if (true == mode_ap) {
                                this->_ap_ssid = buf;
                            } else {
                                this->_sta_ssid = buf;
                            }
                        }
                        break;
                    case 1:
                        if (0 < word.length()) {
                            if (true == mode_ap) {
                                this->_ap_pass = buf;
                            } else {
                                this->_sta_pass = buf;
                            }
                        }
                        break;
                    case 2:
                        if (0 < word.length()) {
                            if (true == mode_ap) {
                                this->_hostname = "";
                            } else {
                                this->_hostname = buf;
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
    this->_default_information_ap();
    this->_default_information_sta();
    result = true;
#endif
    log_d("Load information : MODE[%s] SSID[%s] HOSTNAME[%s]",
          (true == mode_ap) ? "AP" : "STA",
          (true == mode_ap) ? this->_ap_ssid.c_str() : this->_sta_ssid.c_str(),
          (true == mode_ap) ? "" : this->_hostname.c_str());

    return result;
}
bool WebManagerSetting::_save_information(std::string file, std::string ssid, std::string pass, std::string hostname)
{
    bool result = false;
#if SETTING_WIFI_STORAGE_SPI_FS
    log_d("Save information: %s", file.c_str());
    char cip[2 * INPUT_BUFFER_LIMIT] = { 0 };
    uint16_t len;
    File dataFile = SPIFFS.open(file.c_str(), FILE_WRITE);
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
