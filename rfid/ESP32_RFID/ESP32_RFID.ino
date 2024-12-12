#include <Wire.h>
#include <Adafruit_PN532.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_SSD1306.h>


// WiFi credentials
const char* ssid = "Ibu yang Luhur ITS";
const char* password = "5027231025";

// Google Apps Script Web App URL
const String scriptUrl = "https://script.google.com/macros/s/AKfycbxhfHjHc0f38BKM1RTwYO5iQ98n95FSPR99DLmfNVgQnv2QURSsN7Uu3jODGfHr6rZFLQ/exec";

// PN532 I2C setup
#define SDA_PIN 21
#define SCL_PIN 22
Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#define SSD1306_I2C_ADDRESS 0x3C


// GPIO pin for signaling ESP32-CAM
#define GPIO_SIGNAL_PIN 25
#define GREEN_LED_PIN 12   // Green LED pin
#define RED_LED_PIN 13     // Red LED pin

void setup() {
  Serial.begin(115200);

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected!");

  // Initialize PN532
  if (!nfc.begin()) {
    Serial.println("PN532 initialization failed!");
    while (1);
  }
  Serial.println("PN532 initialized.");
  nfc.SAMConfig();

  // Initialize OLED
  if (!display.begin(SSD1306_I2C_ADDRESS, 0x3C)) { // Default OLED I2C address is 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("OLED Initialized");
  display.display();

    // Configure GPIO signaling pin
  pinMode(GPIO_SIGNAL_PIN, OUTPUT);
  digitalWrite(GPIO_SIGNAL_PIN, LOW); // Ensure pin is LOW initially

    // Configure LED GPIO pins
}

void loop() {
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, LOW);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Tempelkan Kartu");
  display.display();

  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;

  // Check if a card is present
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.println("Card detected!");

    // Turn on Green LED to indicate success
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW); // Ensure Red LED is OFF

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Kartu Terdeteksi");
    display.display();

    // Convert UID to string
    String cardId = "";
    for (uint8_t i = 0; i < uidLength; i++) {
      cardId += String(uid[i], HEX);
    }
    Serial.println("Card ID: " + cardId);

    // Send GPIO signal to ESP32-CAM
    sendGpioSignal();
    
    // Send data to Google Apps Script
    if (sendToSpreadsheet(cardId, "Present")) {
      Serial.println("Data posted successfully!");
    } else {
      Serial.println("Failed to post data.");
    }

  delay(2000);  // Prevent duplicate reads
  }
}

bool sendToSpreadsheet(String cardId, String status) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return false;
  }

  HTTPClient http;
  http.begin(scriptUrl);
  http.addHeader("Content-Type", "application/json");

  // Create JSON payload
  String payload = "{\"cardId\":\"" + cardId + "\",\"status\":\"" + status + "\"}";

  // Send POST request
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    Serial.println("Response code: " + String(httpResponseCode));
    String response = http.getString();
    Serial.println("Response: " + response);
  } else {
    Serial.println("Error in sending POST: " + String(httpResponseCode));
  }

  http.end();
  return httpResponseCode == 200;
}

void sendGpioSignal() {
  Serial.println("Sending GPIO signal to ESP32-CAM...");
  digitalWrite(GPIO_SIGNAL_PIN, LOW); // Set GPIO HIGH
  delay(100); // Hold signal for 100ms
  digitalWrite(GPIO_SIGNAL_PIN, HIGH);  // Reset GPIO to LOW
  Serial.println("GPIO signal sent.");
}