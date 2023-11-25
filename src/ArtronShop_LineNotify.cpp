#include "ArtronShop_LineNotify.h"
#include "LINE_Notify_CA.h"
#include <WiFiClientSecure.h>

static const char * TAG = "LINE-Notify";

static String urlEncode(const char *msg) ;
static String urlEncode(String msg) ;

ArtronShop_LineNotify::ArtronShop_LineNotify() {

}

void ArtronShop_LineNotify::begin(String token, Client * client) {
    this->token = token;
    this->client = client;
}

void ArtronShop_LineNotify::setToken(String token) {
    this->token = token;
}

void ArtronShop_LineNotify::setClient(Client * client) {
    this->client = client;
}

bool ArtronShop_LineNotify::send(String massage) {
    return this->send(massage, NULL);
}

bool ArtronShop_LineNotify::send(String massage, LINE_Notify_Massage_Option_t *option) {
    if (massage.length() <= 0) {
        ESP_LOGE(TAG, "massage can't empty");
        return false;
    }

    bool imageUpload = (option->image.data.buffer) && (option->image.data.size > 0);

    // TODO: use user client for Ethernet support
    if (!this->client) {
        this->client = new WiFiClientSecure();
        // ((WiFiClientSecure *) this->client)->setInsecure(); 
        ((WiFiClientSecure *) this->client)->setCACert(LINE_Notify_CA);
    }

    int ret = this->client->connect("notify-api.line.me", 443);
    if (ret <= 0) {
        ESP_LOGE(TAG, "connect to LINE server fail code : %d", ret);
        return false;
    }

    // TODO: support multipart/form-data for upload image
    String payload = "message=" + urlEncode(massage);
    if (option) {
        if (option->sticker.package_id && option->sticker.id) {
            payload += "&stickerPackageId=" + String(option->sticker.package_id);
            payload += "&stickerId=" + String(option->sticker.id);
        }
        if (option->image.url.length() > 0) {
            payload += "&imageFullsize=" + urlEncode(option->image.url);
        }
    }

    this->client->print("POST /api/notify HTTP/1.1\r\n");
    this->client->print("Host: notify-api.line.me\r\n");
    this->client->printf("Authorization: Bearer %s\r\n", this->token.c_str());
    this->client->print("User-Agent: ESP32\r\n");
    this->client->print("Content-Type: application/x-www-form-urlencoded\r\n");
    this->client->printf("Content-Length: %d\r\n", payload.length());
    this->client->print("\r\n");
    this->client->print(payload);

    long timeOut = millis() + 30000;
    while(this->client->connected() && timeOut > millis()) {
        if (this->client->available()) {
            String str = this->client->readString();
            ESP_LOGI(TAG, "%s", str.c_str());
        }
        delay(10);
    }

    return true;
}

// Code from https://github.com/plageoj/urlencode
static String urlEncode(const char *msg) {
  const char *hex = "0123456789ABCDEF";
  String encodedMsg = "";

  while (*msg != '\0') {
    if (
        ('a' <= *msg && *msg <= 'z') || ('A' <= *msg && *msg <= 'Z') || ('0' <= *msg && *msg <= '9') || *msg == '-' || *msg == '_' || *msg == '.' || *msg == '~') {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[(unsigned char)*msg >> 4];
      encodedMsg += hex[*msg & 0xf];
    }
    msg++;
  }
  return encodedMsg;
}

static String urlEncode(String msg) {
  return urlEncode(msg.c_str());
}
