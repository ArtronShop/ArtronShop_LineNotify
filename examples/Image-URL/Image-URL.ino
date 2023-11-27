#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ArtronShop_LineNotify.h>

#define WIFI_SSID "wifi name" // WiFi Name
#define WIFI_PASSWORD "wifi password" // WiFi Password

#define LINE_TOKEN "your LINE token" // LINE Token

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

  LINE.begin(LINE_TOKEN);

  LINE_Notify_Massage_Option_t option;

  // Image support only JPEG and Maximum size is 2048×2048px
  option.image.url = "https://s1145.lnwfile.com/_c/f/_raw/py/fq/dy.jpg";

  if (LINE.send("Hello from ESP32 !", &option)) { // Send "Hello from ESP32 !" and asset to LINE
    Serial.println("Send notify successful");
  } else {
    Serial.printf("Send notify fail. check your token (code: %d)\n", LINE.status_code);
  }
}

void loop() {
  
}
