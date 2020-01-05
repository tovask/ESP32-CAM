

#include <WiFi.h>
// https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html

#define WIFI_AP_SSID "esp32"
#define WIFI_AP_PASS "ESP32-CAM" // should be at least 8 char long
#define WIFI_STA_SSID "PUT_YOUR_WIFI_SSID_HERE"
#define WIFI_STA_PASS "PUT_YOUR_WIFI_PASSWORD_HERE"  // "virslibabszalon"

void wifi_setup() {
  //WiFi.disconnect(true);
  WiFi.mode(WIFI_AP); // WIFI_AP, WIFI_MODE_APSTA

  // Wifi access point
  //WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS); // WiFi.softAP(ssid, password, channel, hidden, max_connection)

  // Wifi station
  //WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASS);

  // shouldn't block the execution here
  // Wait for connection
  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(500);
  //    Serial.print(".");
  //  }
}
