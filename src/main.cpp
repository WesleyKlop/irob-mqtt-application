#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D4
#define RST_PIN D3

MFRC522 reader(SS_PIN, RST_PIN);
int status = 0;
int out = 0;

void setup() {
    Serial.begin(9600);
    SPI.begin();
    reader.PCD_Init();
}

void loop() {

    if (!reader.PICC_IsNewCardPresent()) {
        return;
    }

    if (!reader.PICC_ReadCardSerial()) {
        return;
    }

    Serial.println();
    Serial.print(" UID tag :");
    String content = "";
    for (byte i = 0; i < reader.uid.size; i++) {
        Serial.print(reader.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(reader.uid.uidByte[i], HEX);
        content.concat(String(reader.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(reader.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    Serial.println();
    if (content.substring(1) == "70 38 87 32")
    {
        Serial.println(" Access Granted ");
        Serial.println(" Welcome Mr.Circuit ");
        delay(1000);
        Serial.println(" Have FUN ");
        Serial.println();
        status = 1;
    } else {
        Serial.println(" Access Denied ");
        delay(3000);
    }
}
