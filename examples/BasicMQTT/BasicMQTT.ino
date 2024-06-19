#include <Arduino.h>
#include <CH32Ethernet.h>
#include <CH32EthernetClient.h>
#include <PubSubClient.h>

const char *mqttHostname = "V208_ETH MQTT Test";
const char *mqttHost = "mqttServer";
const char *mqttUser = "user";
const char *mqttPass = "pass";

EthernetClient ethClient;
PubSubClient client(ethClient);
uint32_t mqttLastReconnect = 0;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // handle message arrived
    Serial.printf("MQTT /%s: ", topic);
    Serial.write(payload, length);
    Serial.println();
}

bool mqttReconnect() {
    Serial.print("MQTT connecting... ");
    if (client.connect(mqttHostname, mqttUser, mqttPass)) {
        client.publish("test", "Hello world!");
        client.subscribe("test");
        Serial.println("success!");
    }
    else {
        Serial.println("failed.");
    }
    return client.connected();
}

void mqttLoop() {
    if (!client.connected()) {
        if (millis() - mqttLastReconnect > 5000) {
            mqttLastReconnect = millis();
            mqttReconnect();
        }
    }
    client.loop();
}

void mqttInit() {
    Serial.println("MQTT init");
    client.setServer(mqttHost, 1883);
    client.setCallback(mqttCallback);
    mqttReconnect();
}

void setup() {
    Serial.begin(115200);

    Serial.printf("Initializing Ethernet... ");
    Ethernet.setLedPins(PB0, PB1, true);

    Ethernet.begin();    // DHCP
    // Ethernet.begin(IPAddress(192,168,1,42));    // Static IP

    Serial.printf("done.\n");

    mqttInit();
}

void loop() {
    Ethernet.maintain();
    mqttLoop();
}