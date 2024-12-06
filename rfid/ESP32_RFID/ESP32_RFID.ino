#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 0

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
    Serial.begin(115200);
    Serial.println("Starting setup...");
    delay(100); // Add a delay to ensure system stability
    SPI.begin();
    Serial.println("SPI initialized.");
    rfid.PCD_Init();
    if (rfid.PCD_PerformSelfTest()) {
        Serial.println("RFID reader initialized successfully.");
    } else {
        Serial.println("RFID reader initialization failed.");
    }
}

void loop() {
    Serial.println("Checking for new card...");
    if (!rfid.PICC_IsNewCardPresent()) {
        Serial.println("No new card present.");
        while (!rfid.PICC_IsNewCardPresent()) {
            // Wait until a new card is detected
        }
        MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
        Serial.println("New card detected.");
    }
    

    if (!rfid.PICC_ReadCardSerial()) {
        Serial.println("Failed to read card serial.");
        delay(100); // Increase delay to give more time for reading
        return;
    }
    Serial.println("Card serial read successfully.");

    Serial.print("Card UID: ");
    for (byte i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
    }
    Serial.println();
    Serial.println("RFID card scanned successfully!");

    rfid.PICC_HaltA();
}