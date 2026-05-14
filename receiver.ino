#include <Arduino.h>

#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

// ESP-IDF WiFi includes
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_sleep.h"

#define LED_PIN     2
#define LED_COUNT   7
#define BRIGHTNESS  64

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

volatile bool packetArrived = false;
volatile uint8_t receivedState = 0;
RTC_DATA_ATTR uint8_t savedState = 0;


// ===========================
// ESP-NOW RECEIVE CALLBACK
// ===========================
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    if (len != 1) return;
    receivedState = data[0];
    packetArrived = true;
}

// ===========================
// UPDATE LEDS
// ===========================
void updateLEDs(uint8_t state)
{
    savedState = state;  // persist state across deep sleep

    strip.clear();

    if (state == 1)
    {
        // RED ON
        for (int i = 0; i < LED_COUNT; i++)
            strip.setPixelColor(i, strip.Color(255, 0, 0));
    }
    else
    {
        // GREEN (state 0) = LEDs OFF
        for (int i = 0; i < LED_COUNT; i++)
            strip.setPixelColor(i, strip.Color(0, 0, 0));
    }

    strip.show();
}


// ===========================
// SETUP — runs every boot AND after deep sleep wake
// ===========================
void setup()
{
    Serial.begin(115200);
    uint32_t wakeStart = millis();

    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    updateLEDs(savedState);   // restore last LED state

    // Detect deep-sleep wake
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER)
        Serial.println("Woke from DEEP SLEEP");
    else
        Serial.println("Cold boot");

    // Bring up WiFi for ESP-NOW
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_start();
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

    // Init ESP-NOW
    if (esp_now_init() == ESP_OK)
        esp_now_register_recv_cb(onReceive);

    // Listen for 100 ms
    uint32_t start = millis();
    while (millis() - start < 100)
    {
        if (packetArrived)
        {
            packetArrived = false;
            updateLEDs(receivedState);
        }
    }

    // Shutdown WiFi fully
    esp_wifi_stop();
    WiFi.mode(WIFI_OFF);

    // Disable all unwanted wake sources
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

    // Timer wake after 10 seconds
    esp_sleep_enable_timer_wakeup(10ULL * 1000ULL * 1000ULL);

    // Prevent USB subsystem from waking device instantly
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);

    Serial.println("Entering DEEP SLEEP for 10 seconds...");
    Serial.print("Awake time (ms): ");
    Serial.println(millis() - wakeStart);
    delay(50);
    esp_deep_sleep_start();
}

void loop()
{
    // Never reached — deep sleep always reboots into setup()
}
