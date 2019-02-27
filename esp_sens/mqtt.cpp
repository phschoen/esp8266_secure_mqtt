#include <PubSubClient.h>
#include "wifi.h"
#include "mqtt.h"
#include "config.h"

PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}


void reconnect() {
  int tries=3;
  // Loop until we're reconnected
  while (!client.connected() && tries > 0) {
    tries--;
    Serial.print("Attempting MQTT connection...");
    
    // Create a random client ID
    String clientId = MQTT_USER;
    clientId += "-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PW)) {
      Serial.println("connected");
      
      if (espClient.verify(MQTT_SERVER_FINGERPRINT, MQTT_SERVER)) {
        Serial.println("certificate matches");
      } else {
        Serial.println("certificate doesn't match");
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

#include <FS.h>
// Load Certificates
void loadcerts() {

  if (!SPIFFS.begin()) {
   Serial.println("Failed to mount file system");
   return;
 }

 // Load client certificate file from SPIFFS
 File cert = SPIFFS.open("/esp.der", "r"); //replace esp.der with your uploaded file name
 if (!cert) {
   Serial.println("Failed to open cert file");
 }
 else
   Serial.println("Success to open cert file");

 delay(1000);

 // Set client certificate
 if (espClient.loadCertificate(cert))
   Serial.println("cert loaded");
 else
   Serial.println("cert not loaded");

 // Load client private key file from SPIFFS
 File private_key = SPIFFS.open("/espkey.der", "r"); //replace espkey.der with your uploaded file name
 if (!private_key) {
   Serial.println("Failed to open private cert file");
 }
 else
   Serial.println("Success to open private cert file");

 delay(1000);

 // Set client private key
 if (espClient.loadPrivateKey(private_key))
   Serial.println("private key loaded");
 else
   Serial.println("private key not loaded");


 // Load CA file from SPIFFS
 File ca = SPIFFS.open("/ca.der", "r"); //replace ca.der with your uploaded file name
 if (!ca) {
   Serial.println("Failed to open ca ");
 }
else
  Serial.println("Success to open ca");
  delay(1000);

  // Set server CA file
   if(espClient.loadCACert(ca))
   Serial.println("ca loaded");
   else
   Serial.println("ca failed");

}

void verifytls() {
  // Use WiFiClientSecure class to create TLS connection
  Serial.print("connecting to ");
  Serial.println(MQTT_SERVER);
  if (!espClient.connect(MQTT_SERVER, 8883)) {
    Serial.println("connection failed");
    return;
  }
}
