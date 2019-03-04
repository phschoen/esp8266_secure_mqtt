
#include <PubSubClient.h>

extern PubSubClient mqtt_client;

void loadcerts();

void mqtt_callback(char* topic, byte* payload, unsigned int length);

void mqtt_reconnect();

void verifytls();
