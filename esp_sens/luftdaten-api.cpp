
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "debug.h"
#include "luftdaten-api.h"

const struct api madavi = {
  .host = "api-rrd.madavi.de",
  .url = "/data.php",
  .port = 443,
};

const struct api luftdaten = {
  .host = "api.luftdaten.info",
  .url = "/v1/push-sensor-data/",
  .port = 443,
};

const struct api sensemap = {
  .host = "ingress.opensensemap.org",
  .url = "/boxes/BOXID/data?luftdaten=1",
  .port = 443,
};

const char TXT_CONTENT_TYPE_JSON[] PROGMEM = "application/json";
const char TXT_CONTENT_TYPE_INFLUXDB[] PROGMEM = "application/x-www-form-urlencoded";
const char TXT_CONTENT_TYPE_TEXT_HTML[] PROGMEM = "text/html; charset=utf-8";
const char TXT_CONTENT_TYPE_TEXT_PLAIN[] PROGMEM = "text/plain";
const char TXT_CONTENT_TYPE_IMAGE_SVG[] PROGMEM = "image/svg+xml";
const char TXT_CONTENT_TYPE_IMAGE_PNG[] PROGMEM = "image/png";



uint8_t type_to_pin(const enum sensor_types type) {
  switch (type) {
        case SDS:
        case PPD:
        case HPM:
            return 1;

        case BMP:
            return 3;

        case PMS:
            return 5;

        case GPS:
            return 9;

        case DHT:
        case HTU21D:
            return 7;

        case BME280:
            return 11;

        case DS18B20:
            return 13;
    }
}



void sendLuftdaten(
        uint64_t esp_chipid,
        const char* sensor_result,
        const enum sensor_types type,
        const struct api* api) {
  uint16_t json_str_len = 1024;
  uint16_t off = 0;
  char* json_str = (char*) malloc(json_str_len);
  
  off += snprintf(json_str+off, json_str_len-off,"{ ");
  off += snprintf(json_str+off, json_str_len-off,"\"software_version\": \""SOFTWARE_VERSION"\",");
  off += snprintf(json_str+off, json_str_len-off,"\"sensordatavalues\":[%s] ",sensor_result);
  off += snprintf(json_str+off, json_str_len-off,"}");

  int pin = type_to_pin(type);
  sendData(esp_chipid, json_str, pin, api, NULL, "application/json");
  free(json_str);
}

void sendData(
        uint64_t esp_chipid,
        const char* data,
        const int pin,
        const struct api* api,
        const char* basic_auth_string,
        const char* contentType)
{

  uint16_t request_size = 1024;
  uint16_t off = 0;
  char* request_head = (char*) malloc(request_size);
  
  off += snprintf(request_head+off, request_size-off,"POST %s HTTP/1.1\r\n", api->url);
  off += snprintf(request_head+off, request_size-off,"Host: %s\r\n ", api->host);
  off += snprintf(request_head+off, request_size-off,"Content-Type: %s\r\n", contentType);

  if (basic_auth_string != NULL) {
    off += snprintf(request_head+off, request_size-off,"Authorization: Basic %s \r\n", basic_auth_string);
  }

  off += snprintf(request_head+off, request_size-off,"X-PIN: %d \r\n", pin);

#if defined(ESP32)
  off += snprintf(request_head+off, request_size-off,"X-Sensor: esp32-%llu\r\n", esp_chipid);
#else
  off += snprintf(request_head+off, request_size-off,"X-Sensor: esp8266-%llu\r\n", esp_chipid);
#endif

  off += snprintf(request_head+off, request_size-off,"Content-Length: %d\r\n", strlen(data));
  //request_head += "Content-Length: " + String(data.length(), DEC) + String("\r\n");
  off += snprintf(request_head+off, request_size-off,"Connection: close\r\n\r\n");
  off += snprintf(request_head+off, request_size-off,"\r\n");

  // Use WiFiClient class to create TCP connections
//  if (api->port == 443) {
//    WiFiClientSecure client_s;
//
//    client_s.setNoDelay(true);
//    client_s.setTimeout(20000);
//
//    //if (!client_s.connect(api->host, api->port))
//    {
//      debug_error("connection failed\n");
//      //return;
//    }
//
//    //    if (client_s.verifyCertChain(host)) {
//    //      debug_info("Server certificate verified\n");
//    //    } else {
//    //      debug_info("ERROR: certificate verification failed!\n");
//    //      return;
//    //    }
//
//    debug_info("Requesting URL: %s\n", api->url);
//    debug_info("chip id %d\n", esp_chipid);
//    debug_info("Request Header:\n%s\n",request_head);
//    debug_info("Request Data:\n%s\n",data);
//
//    // send request to the server
//    //client_s.print(request_head);
//    //client_s.println(data);
//    delay(10);
//
//    // Read reply from server and print them
//    while (client_s.available()) {
//      char c = client_s.read();
//      debug_info(c);
//    }
//  } else 
    {
    WiFiClient client;

    client.setNoDelay(true);
    client.setTimeout(20000);

    if (!client.connect(api->host, api->port)) {
      debug_error("connection failed\n");
      return;
    }


    debug_info("Requesting URL: %s\n", api->url);
    debug_info("chip id %d\n", esp_chipid);
    debug_info("Request Header:\n%s\n",request_head);
    debug_info("Request Data:\n%s\n",data);
    
    client.print(request_head);

    client.println(data);

    delay(10);

    // Read reply from server and print them
    while (true) {
      char tmp[512];
      uint16_t i=0;
      while (client.available()) {
        tmp[i] = client.read();
        i++;
        if(i==sizeof(tmp)-1)
          break;
      }
      tmp[i]='\0';
      debug_info("%s",tmp);
      if(!client.available())
        break;
    }
  }
  debug_info("closing connection\n");

  debug_info("End connecting to %s \n", api->host);

#if not defined(ESP32)
  //wdt_reset(); // nodemcu is alive
  //yield();
#endif
}
