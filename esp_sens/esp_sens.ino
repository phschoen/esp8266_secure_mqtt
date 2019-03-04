#include <Wire.h>
#include "SdsDustSensor.h"
#include "luftdaten-api.h"

#include "SHTSensor.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"


int rxPin = D1;
int txPin = D2;
SdsDustSensor sds(rxPin, txPin);

#define SDA_PIN D3
#define SCL_PIN D4
SHTSensor sht;
// To use a specific sensor instead of probing the bus use this command:
// SHTSensor sht(SHTSensor::SHT3X);
#define MINUTES_SLEEP 1

uint64_t esp_chipid;

void setup() {

  Wire.begin(SDA_PIN, SCL_PIN);        // join i2c bus (address optional for master)
  Serial.begin(115200);
  delay(1000); // let serial console settle


  // sds init
  esp_chipid = ESP.getChipId();
  debug_info("id is %llu\n", esp_chipid);
  debug_info("Firmware of sds %s\n", sds.queryFirmwareVersion().toString().c_str()); // prints firmware version
  debug_info("mode %s\n", sds.setActiveReportingMode().toString().c_str()); // ensures sensor is in 'active' reporting mode
  
  sds.begin();
  Serial.println(sds.setCustomWorkingPeriod(MINUTES_SLEEP).toString()); // sensor sends data every 1 minutes
  //Serial.println(sds.setContinuousWorkingPeriod().toString()); // ensures sensor has continuous working period - default but not recommended

  // sht init 
  if (sht.init()) {
      Serial.print("init(): success\n");
  } else {
      Serial.print("init(): failed\n");
  }
  sht.setAccuracy(SHTSensor::SHT_ACCURACY_HIGH); // only supported by SHT3x
  setup_wifi();

  //mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt_client.setCallback(mqtt_callback);
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

  debug_info("setup done\n");
}

long lastMsg = 0;
void loop() {
  debug_info("enter loop\n");

  mqtt_client.loop();
  long now = millis();
  
  PmResult pm = sds.readPm();
  if (pm.isOk()) {
    debug_info("PM2.5 = %f", pm.pm25);
    debug_info(", PM10 = %f\n", pm.pm10);

    // if you want to just print the measured values, you can use toString() method as well
    debug_info(pm.toString().c_str());
  } else {
    if (now -lastMsg > MINUTES_SLEEP * 60 * 1000 *2 ) {
      Serial.print("error no data from dust sensor for a long time\n");
      ESP.reset();
      return;
    }
    // notice that loop delay is set to 0.5s and some reads are not available
    //Serial.print("Could not read values from sensor, reason: ");
    //Serial.println(pm.statusToString());
    //delay(MINUTES_SLEEP * 60 * 1000 * 0.5);
    return;
  }
  if (sht.readSample()) {
      Serial.print("SHT:\n");
      Serial.print("  RH: ");
      Serial.print(sht.getHumidity(), 2);
      Serial.print("\n");
      Serial.print("  T:  ");
      Serial.print(sht.getTemperature(), 2);
      Serial.print("\n");
  } else {
      Serial.print("Error in readSample()\n");
  }
  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }
  
  if (now - lastMsg > 2000) {
    lastMsg = now;
    char msg[50];
    
    snprintf (msg, sizeof(msg), "%f", sht.getHumidity());
    Serial.print("Publish message: "); Serial.println(msg);
    mqtt_client.publish("/balcony/humidity", msg);
    
    snprintf (msg, sizeof(msg), "%f", sht.getTemperature());
    Serial.print("Publish message: "); Serial.println(msg);
    mqtt_client.publish("/balcony/temperature", msg);
    
    snprintf (msg, sizeof(msg), "%f", pm.pm10);
    Serial.print("Publish message: "); Serial.println(msg);
    mqtt_client.publish("/balcony/dust_pm10", msg);
    
    snprintf (msg, sizeof(msg), "%f", pm.pm25);
    Serial.print("Publish message: ");Serial.println(msg);
    mqtt_client.publish("/balcony/dust_pm25", msg);
  }

}
