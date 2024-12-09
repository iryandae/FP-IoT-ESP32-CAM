#include <Wire.h>
#include <Adafruit_PN532.h>

// Pin SDA dan SCL untuk ESP32 (standar I²C pin)
#define SDA_PIN 21
#define SCL_PIN 22

// Inisialisasi PN532 menggunakan I²C
Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN); // Inisialisasi I²C

  Serial.println("Inisialisasi modul PN532...");

  if (!nfc.begin()) {
    Serial.println("Gagal mendeteksi modul PN532. Periksa koneksi Anda!");
    while (1); // Berhenti jika modul tidak terdeteksi
  }

  // Set mode untuk membaca kartu NFC
  nfc.SAMConfig();
  Serial.println("PN532 siap! Tempelkan kartu NFC Anda.");
}

void loop() {
  uint8_t success;
  uint8_t uid[7]; // Array untuk menyimpan UID
  uint8_t uidLength;

  // Cek apakah ada kartu yang terdeteksi
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.println("Kartu terdeteksi!");

    // Tampilkan UID kartu
    Serial.print("UID: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(uid[i], HEX);
      if (i < uidLength - 1) {
        Serial.print(":");
      }
    }
    Serial.println();

    // Tunggu sebentar sebelum membaca kartu berikutnya
    delay(2000);
  }
}
