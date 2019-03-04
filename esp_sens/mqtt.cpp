#include <PubSubClient.h>
#include "wifi.h"
#include "mqtt.h"
#include "config.h"
#include "debug.h"

PubSubClient mqtt_client(MQTT_SERVER, MQTT_PORT, espClient);

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  debug_info("Message arrived [");
  debug_info(topic);
  debug_info("] \n");
  for (int i = 0; i < length; i++) {
    debug_info("%*s\n",length,(char*)payload);
  }
  debug_info("\n");

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}


void mqtt_reconnect() {
  int tries=5;
  // Loop until we're reconnected
  while (!mqtt_client.connected() && tries > 0) {
    tries--;
    debug_info("Attempting MQTT connection...\n");
    
    // Create a random client ID
    String clientId = "esp";
    clientId += "-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    #ifdef MQTT_USER
    //int rt = mqtt_client.connect(clientId.c_str(), MQTT_USER, MQTT_PW);
    #else
    int rt = mqtt_client.connect(clientId.c_str());
    #endif
    if (rt) {
      debug_info("connected\n");

      #ifdef MQTT_SERVER_FINGERPRINT
        if (espClient.verify(MQTT_SERVER_FINGERPRINT, MQTT_SERVER)) {
          debug_info("certificate matches\n");
        } else {
          debug_info("certificate doesn't match\n");
        }
      #endif
    } else {
      debug_info("failed, rc=%d\n", mqtt_client.state());
      debug_info(" try again in 5 seconds\n");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  if (tries <=0) {
    debug_info("failed to connect: rest ESP\n", mqtt_client.state());
    ESP.reset();
  }
}

#include <FS.h>
// Load Certificates
void loadcerts() {

 #ifdef MQTT_SERVER_FINGERPRINT
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
   #endif

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
