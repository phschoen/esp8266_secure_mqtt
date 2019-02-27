#include <Wire.h>
#include "SdsDustSensor.h"

#include "SHTSensor.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"


int rxPin = D1;
int txPin = D2;
SdsDustSensor sds(rxPin, txPin);

#define SDA_PIN D3
#define SCL_PIN D4
SHTSensor sht;
// To use a specific sensor instead of probing the bus use this command:
// SHTSensor sht(SHTSensor::SHT3X);
#define MINUTES_SLEEP 1


void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);        // join i2c bus (address optional for master)
  Serial.begin(9600);
  delay(1000); // let serial console settle

  // sds init
  sds.begin();
  Serial.println(sds.queryFirmwareVersion().toString()); // prints firmware version
  Serial.println(sds.setActiveReportingMode().toString()); // ensures sensor is in 'active' reporting mode
  
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

  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output

}

long lastMsg = 0;
void loop() {
  PmResult pm = sds.readPm();
  if (pm.isOk()) {
    Serial.print("PM2.5 = ");
    Serial.print(pm.pm25);
    Serial.print(", PM10 = ");
    Serial.println(pm.pm10);

    // if you want to just print the measured values, you can use toString() method as well
    Serial.println(pm.toString());
  } else {
    // notice that loop delay is set to 0.5s and some reads are not available
    //Serial.print("Could not read values from sensor, reason: ");
    //Serial.println(pm.statusToString());
    delay(MINUTES_SLEEP * 60 * 1000 * 0.5);
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
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  
  if (now - lastMsg > 2000) {
    lastMsg = now;
    char msg[50];
    
    snprintf (msg, sizeof(msg), "%ld", sht.getHumidity());
    Serial.print("Publish message: "); Serial.println(msg);
    client.publish("outdoor/balcony/humidity", msg);
    
    snprintf (msg, sizeof(msg), "%ld", sht.getTemperature());
    Serial.print("Publish message: "); Serial.println(msg);
    client.publish("outdoor/balcony/temperature", msg);
    
    snprintf (msg, sizeof(msg), "%ld", pm.pm10);
    Serial.print("Publish message: "); Serial.println(msg);
    client.publish("outdoor/balcony/dust_pm10", msg);
    
    snprintf (msg, sizeof(msg), "%ld", pm.pm25);
    Serial.print("Publish message: ");Serial.println(msg);
    client.publish("outdoor/balcony/dust_pm25", msg);
  }

}
