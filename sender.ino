#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_NeoPixel.h>
#include "esp_wifi.h"

// ----------------------
// Hardware Setup
// ----------------------
#define BUTTON_PIN 4
#define NEOPIXEL_PIN 48     // ⭐ BUILT-IN LED ON ESP32-S3
#define NUM_PIXELS 1

Adafruit_NeoPixel pixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ----------------------
// ESP-NOW receiver MAC
// (replace with whichever device is the receiver)
// ----------------------
uint8_t receiverMAC[] = { 0x14, 0x08, 0x08, 0xA5, 0xA8, 0xA8 };



// ----------------------
uint8_t currentState = 0;   // 0 = GREEN, 1 = RED


// =====================================================================
// ESP-NOW send callback — REQUIRED SIGNATURE FOR ESP32-S3 (Core 3.x)
// =====================================================================
void onSent(const wifi_tx_info_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}


// =====================================================================
// Update the onboard NeoPixel to show the state
// =====================================================================
void updatePixel() {
  pixel.clear();
  if (currentState == 0)
    pixel.setPixelColor(0, pixel.Color(20, 255, 20));  // GREEN
  else
    pixel.setPixelColor(0, pixel.Color(255, 60, 40));  // RED

  pixel.show();
}


// =====================================================================
// SETUP
// =====================================================================
void setup()
{
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pixel.begin();
  pixel.setBrightness(64);
  updatePixel();

  // -------- WiFi Init --------
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_start();

  // Lock channel 1 (receiver must match)
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  // -------- ESP-NOW Init --------
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    while (true);
  }

  esp_now_register_send_cb(onSent);

  // Add receiver peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer!");
    while (true);
  }

  Serial.println("Sender ready.");
}


// =====================================================================
// LOOP — blast packets for 5 seconds when button pressed
// =====================================================================
void loop()
{
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    // Toggle state
    currentState ^= 1;
    updatePixel();

    Serial.println("Button pressed — blasting packets...");

    // 5-second broadcast, every 25 ms (best reliability)
    uint32_t start = millis();
    while (millis() - start < 11000)
    {
      esp_now_send(receiverMAC, &currentState, sizeof(currentState));
      delay(25);
    }

    Serial.println("Blast complete.");
  }
}
