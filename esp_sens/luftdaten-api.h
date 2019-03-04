#ifndef __LUFTDATEN_API_H__
#define __LUFTDATEN_API_H__

#include <Arduino.h>


#define SOFTWARE_VERSION "NRZ-2018-xxx_ESP32"

struct api {
    const char* host;
    const char* url;
    uint16_t port;
};
extern const struct api madavi;
extern const struct api luftdaten;
extern const struct api sensemap;

enum sensor_types {
    HTU21D, // HTU21D, temperature, humidity
    DHT,    // DHT22, temperature, humidity
    HPM, // HTU21D, temperature, humidity
         // Honeywell PM sensor

    BME280, // BME280, temperature, humidity, pressure

    BMP, // BMP180, temperature, pressure

    DS18B20, // DS18B20, temperature

    GPS, // GPS, bevorzugt Neo-6M

    PMS, // all Plantower (PMS) sensors
    SDS, // dust sensors like SDS011
    PPD, // dust sensor cheap like PPD42NS
};

uint8_t type_to_pin(const enum sensor_types type);

void sendLuftdaten(
        uint64_t esp_chipid,
        const char* sensor_result,
        const enum sensor_types type,
        const struct api* api);
void sendData(
        uint64_t esp_chipid,
        const char*  data,
        const int pin,
        const struct api* api,
        const char* basic_auth_string,
        const char* contentType);
#endif
