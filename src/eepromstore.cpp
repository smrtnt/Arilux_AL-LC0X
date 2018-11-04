#include "eepromstore.h"

#include <string.h>
#include <ESP_EEPROM.h>
#include "debug.h"

#define HSB_START_ADDRESS 0

EEPromStore::EEPromStore(const uint16_t p_eepromAddress) :
    m_eepromAddress(p_eepromAddress) {
}

blobData_t EEPromStore::getBlob() const {
    blobData_t data;
    uint16_t storedCRC = 0;
    EEPROM.get(m_eepromAddress, data);
    EEPROM.get(m_eepromAddress + sizeof(blobData_t), storedCRC);
    const uint16_t calculcatedCRC = crc16(reinterpret_cast<uint8_t*>(&data), sizeof(blobData_t));

    if (storedCRC == calculcatedCRC) {
        DEBUG_PRINTLN(F("EEPromStore : CRC matched "));
        return data;
    } else {
        DEBUG_PRINTLN(F("EEPromStore : CRC mismatch "));
        return {0, 0, 0, 0, 0, 0, 0};
    }
}

void EEPromStore::save(const SettingsDTO& settings) {
    const HSB hsb = settings.hsb();
    blobData_t data;
    data.m_hue = hsb.hue();
    data.m_saturation = hsb.saturation();
    data.m_brightness = hsb.brightness();
    data.m_white1 = hsb.white1();
    data.m_white2 = hsb.white2();

    data.m_filters = 0;
    data.m_remoteBase = settings.remoteBase();
    data.m_power = settings.power();
    data.m_onBrightness = settings.brightness();

    storeBlob(data);
    DEBUG_PRINTLN(F("EEPromStore : HSB "));
}

SettingsDTO EEPromStore::get() const {
    blobData_t data;
    uint16_t storedCRC = 0;
    EEPROM.get(m_eepromAddress, data);
    EEPROM.get(m_eepromAddress + sizeof(blobData_t), storedCRC);
    const uint16_t calculcatedCRC = crc16(reinterpret_cast<uint8_t*>(&data), sizeof(blobData_t));

    if (storedCRC == calculcatedCRC) {
        DEBUG_PRINTLN(F("EEPromStore : CRC match "));
        return SettingsDTO(
                   HSB(data.m_hue, data.m_saturation, data.m_brightness, data.m_white1, data.m_white2),
                   data.m_remoteBase,
                   0,
                   data.m_power,
                   data.m_onBrightness);
    } else {
        DEBUG_PRINT(F("EEPromStore : CRC mismatch "));
        return SettingsDTO();
    }
}

void EEPromStore::storeBlob(const blobData_t& p_blob) const {
    blobData_t data = p_blob;
    uint16_t crc = crc16(reinterpret_cast<uint8_t*>(&data), sizeof(blobData_t));
    EEPROM.put(m_eepromAddress, p_blob);
    EEPROM.put(m_eepromAddress + sizeof(blobData_t), crc);
    DEBUG_PRINTLN(F("EEPromStore : Commit"));
}

uint16_t EEPromStore::crc16(uint8_t* a, uint16_t length) const {
    uint16_t crc = 0;

    for (uint16_t i = 0; i < length; i++) {
        crc = crc16Update(crc, a[i]);
    }

    return crc;
}

uint16_t EEPromStore::crc16Update(uint16_t crc, uint8_t a) const {
    int i;
    crc ^= a;

    for (i = 0; i < 8; ++i)  {
        if (crc & 1) {
            crc = (crc >> 1) ^ 0xA001;
        } else {
            crc = (crc >> 1);
        }
    }

    return crc;
}