#include "ArtronShop_LineNotify.h"
#include "LINE_Notify_CA.h"
#ifdef ARDUINO_UNOWIFIR4
#include <WiFiSSLClient.h>
#else
#include <WiFiClientSecure.h>
#endif

#ifdef ARDUINO_UNOWIFIR4
#define ESP_LOGI(...) ;
#define ESP_LOGE(...) ;
#endif

static const char * TAG = "LINE-Notify";

static String urlEncode(const char *msg) ;
static String urlEncode(String msg) ;

ArtronShop_LineNotify::ArtronShop_LineNotify() {

}

void ArtronShop_LineNotify::begin(String token, Client *client) {
    this->token = token;
    this->client = client;
}

void ArtronShop_LineNotify::setToken(String token) {
    this->token = token;
}

void ArtronShop_LineNotify::setClient(Client *client) {
    this->client = client;
}

bool ArtronShop_LineNotify::send(String massage, LINE_Notify_Massage_Option_t *option) {
    if (massage.length() <= 0) {
        ESP_LOGE(TAG, "massage can't empty");
        return false;
    }

    bool imageUpload = false;

    // TODO: use user client for Ethernet support
    if (!this->client) {
#ifdef ARDUINO_UNOWIFIR4
        this->client = new WiFiSSLClient();
#else
        this->client = new WiFiClientSecure();
        // ((WiFiClientSecure *) this->client)->setInsecure(); 
        ((WiFiClientSecure *) this->client)->setCACert(LINE_Notify_CA);
#endif
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
            payload += "&imageThumbnail=" + urlEncode(option->image.url);
            payload += "&imageFullsize=" + urlEncode(option->image.url);
        }
        if (option->map.lat && option->map.lng) {
            if (option->map.service == LONGDO_MAP) {
                String map_url = "https://mmmap15.longdo.com/mmmap/snippet/index.php?width=1000&height=1000";
                map_url += "&lat=" + String(option->map.lat, 9);
                map_url += "&long=" + String(option->map.lng, 9);
                map_url += "&zoom=" + String(option->map.zoom);
                map_url += "&pinmark=" + String(option->map.noMaker ? '0' : '1');
                if (option->map.option.length() > 0) {
                    map_url += "&" + option->map.option;
                }
                map_url += "&HD=1";

                ESP_LOGI(TAG, "Map image URL: %s", map_url.c_str());

                payload += "&imageThumbnail=" + urlEncode(map_url);
                payload += "&imageFullsize=" + urlEncode(map_url);
            } else if (option->map.service == GOOGLE_MAP) {
                String map_url = "https://maps.googleapis.com/maps/api/staticmap";
                map_url += "?center=" + String(option->map.lat, 9) + "," + String(option->map.lng, 9);
                map_url += "&markers=color:red%7Clabel:U%7C" + String(option->map.lat, 9) + "," + String(option->map.lng, 9);
                map_url += "&zoom=" + String(option->map.zoom);
                map_url += "&size=1000x1000";
                map_url += "&format=jpg";
                map_url += "&key=" + option->map.api_key;
                if (option->map.option.length() > 0) {
                    map_url += "&" + option->map.option;
                }

                ESP_LOGI(TAG, "Map image URL: %s", map_url.c_str());

                payload += "&imageThumbnail=" + urlEncode(map_url);
                payload += "&imageFullsize=" + urlEncode(map_url);
            }
        }
        imageUpload = (option->image.data.buffer) && (option->image.data.size > 0);
    }

    this->client->print("POST /api/notify HTTP/1.1\r\n");
    this->client->print("Host: notify-api.line.me\r\n");
    this->client->print("Authorization: Bearer " + this->token + "\r\n");
    this->client->print("User-Agent: ESP32\r\n");
    this->client->print("Content-Type: application/x-www-form-urlencoded\r\n");
    this->client->print("Content-Length: " + String(payload.length()) + "\r\n");
    this->client->print("\r\n");
    this->client->print(payload);

    delay(20); // wait server respond

    long timeout = millis() + 30000;
    bool first_line = true;
    int state = 0;
    while(this->client->connected() && (timeout > millis())) {
        if (this->client->available()) {
            if (state == 0) { // Header
                String line = this->client->readStringUntil('\n');
                if (line.endsWith("\r")) {
                    line = line.substring(0, line.length() - 1);
                }
                ESP_LOGI(TAG, "Header: %s", line.c_str());
                if (first_line) {
                    if (sscanf(line.c_str(), "HTTP/%*f %d", &this->status_code) >= 1) {
                        first_line = false;
                    } else {
                        ESP_LOGE(TAG, "invalid first line");
                    }
                } else {
                    // Header
                    if (line.length() == 0) {
                        state = 2;
                    }
                }
            } else if (state == 2) { // Data
                String line = this->client->readStringUntil('\n');
                if (line.endsWith("\r")) {
                    line = line.substring(0, line.length() - 1);
                }
                ESP_LOGI(TAG, "Data: %s", line.c_str());
                if (line.length() == 0) {
                    break;
                }
            }
        }
        delay(10);
    }
    ESP_LOGI(TAG, "END");
    
    this->client->stop();
    delete this->client;
    this->client = NULL;

    return this->status_code == 200;
}

ArtronShop_LineNotify LINE;

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
