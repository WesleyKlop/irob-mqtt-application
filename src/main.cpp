#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>

#include "secrets.h"

#define SS_PIN D4
#define RST_PIN D3

MFRC522 reader(SS_PIN, RST_PIN);
WiFiClient wifiClient;
ESP8266WiFiMulti wifi;
PubSubClient mqtt(wifiClient);

const char *mqtt_host = "192.168.178.10";
const char *mqtt_client_name = "esp8266-client-01";

void reconnect_mqtt() {
    while (!mqtt.connected()) {
        if (!mqtt.connect(mqtt_client_name, "esp8266", "irob-is-cool")) {
            Serial.print("Failed to connect to mqtt: ");
            Serial.print(mqtt.state());
            delay(1000);
        } else {
            Serial.println("(Re)connected to mqtt broker");
            mqtt.subscribe("door/state");
            mqtt.publish("hello", mqtt_client_name);
        }
    }
}

void setup() {
    wifi.addAP(WIFI_SSID, WIFI_PASS);
    Serial.begin(9600);
    SPI.begin();

    // Configure wifi
    while (wifi.run() != WL_CONNECTED) {
        delay(1000);
        Serial.print('.');
    }
    Serial.print(" Obtained IP: ");
    Serial.println(WiFi.localIP());

    // Configure rfid reader
    reader.PCD_Init();

    // Configure mqtt client
    mqtt.setServer(mqtt_host, 1883);
    mqtt.setCallback([](char *topic, byte *payload, unsigned int length) {
        Serial.println(topic);
        Serial.println((char *) payload);
    });

    reconnect_mqtt();
}

String lastCard;

void loop() {
    reconnect_mqtt();
    if (!reader.PICC_IsNewCardPresent()) {
        return;
    }

    if (!reader.PICC_ReadCardSerial()) {
        return;
    }

    String content = "";
    for (byte i = 0; i < reader.uid.size; i++) {
        content.concat(String(reader.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(reader.uid.uidByte[i], HEX));
    }
    content.toUpperCase();

    // Make sure we don't DoS our broker
    if (lastCard == content) {
        return;
    }
    Serial.println();
    Serial.print("Received tag with uid: ");
    Serial.println(content);
    Serial.println("Requesting access...");

    mqtt.publish("door/access", content.c_str());
    lastCard = content;
}
