#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ArtronShop_LineNotify.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define LINE_TOKEN ""

WiFiMulti wifiMulti;

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  // wait for WiFi connection
  Serial.print("Waiting for WiFi to connect...");
  while ((wifiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
  }
  Serial.println(" connected");

  Notify.begin(LINE_TOKEN);
  if (Notify.send("Hello from ESP32 !")) { // Send "Hello from ESP32 !" to LINE
    Serial.println("Send notify successful");
  } else {
    Serial.println("Send notify fail. check your token");
  }
}

void loop() {
  
}