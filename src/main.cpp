#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include <Servo.h>

#include "secrets.h"

#define SERVO_PIN D2
#define RST_PIN D3
#define SS_PIN D4

MFRC522 reader(SS_PIN, RST_PIN);
WiFiClient wifiClient;
ESP8266WiFiMulti wifi;
PubSubClient mqtt(wifiClient);

Servo servo;
const char *mqtt_host = "192.168.178.10";
const char *mqtt_client_name = "esp8266-client-01";

String lastCard;

void ensure_mqtt_connection() {
    while (!mqtt.connected()) {
        if (!mqtt.connect(mqtt_client_name, "esp8266", "irob-is-cool")) {
            Serial.print("Failed to connect to mqtt: ");
            Serial.print(mqtt.state());
            delay(1000);
        } else {
            Serial.println("(Re)connected to mqtt broker");
            if (mqtt.subscribe("door/state") == false) {
                Serial.println("Could not subscribe to topic 'door/state'");
            }
            mqtt.publish("hello", mqtt_client_name);
        }
    }
}

void mqtt_callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    String message = "";
    for (int i = 0; i < length; i++) {
        message.concat((char)payload[i]);
    }
    Serial.println(message);
    if (message == "open") {
        Serial.println("OPEN");
        servo.write(180);
    }
    if (message == "close") {
        Serial.println("CLOSE");
        servo.write(0);
        lastCard = "";
    }
}

void setup() {
    wifi.addAP(WIFI_SSID, WIFI_PASS);
    Serial.begin(9600);
    SPI.begin();

    // Configure servo
    servo.attach(SERVO_PIN);

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
    mqtt.setCallback(mqtt_callback);

    ensure_mqtt_connection();
}

void loop() {
    ensure_mqtt_connection();
    mqtt.loop();

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
    content.trim();
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
