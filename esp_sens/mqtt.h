
#include <PubSubClient.h>

extern PubSubClient client;

void loadcerts();

void callback(char* topic, byte* payload, unsigned int length);

void reconnect();

void verifytls();
