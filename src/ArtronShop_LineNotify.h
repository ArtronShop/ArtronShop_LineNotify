#ifndef __ARTRONSHOP_LINE_NOTIFY__
#define __ARTRONSHOP_LINE_NOTIFY__

#include <Arduino.h>
#ifdef ARDUINO_UNOWIFIR4
#include <WiFiS3.h>
#else
#include <WiFi.h>
#include "FS.h"
#endif

typedef enum {
    LONGDO_MAP,
    GOOGLE_MAP
} LINE_Notify_MAP_Service_t;

typedef struct {
    // sticker
    struct {
        int package_id = 0;
        int id = 0;
    } sticker;

    // image
    struct {
        // image from internet
        String url;

        // image from flash / RAM
        struct {
            void *buffer = NULL;
            size_t size = 0;
        } data;

        // image form external storage
#ifndef ARDUINO_UNOWIFIR4
        File file;
#endif
    } image;

    // map
    struct {
        LINE_Notify_MAP_Service_t service = LONGDO_MAP;
        float lat = 0;
        float lng = 0;
        int zoom = 20;
        bool noMaker = false;
        String api_key;
        String option;
    } map;
} LINE_Notify_Massage_Option_t;


class ArtronShop_LineNotify {
    private:
        String token = "";
        Client *client = NULL;
 
    public:
        int status_code = 0;
        
        ArtronShop_LineNotify() ;

        void begin(String token = "", Client *client = NULL) ;
        void setToken(String token) ;
        void setClient(Client *client) ;
        bool send(String massage, LINE_Notify_Massage_Option_t *option = NULL) ;

};

extern ArtronShop_LineNotify LINE;

#endif