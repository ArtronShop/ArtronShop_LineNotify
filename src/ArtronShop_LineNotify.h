#ifndef __ARTRONSHOP_LINE_NOTIFY__
#define __ARTRONSHOP_LINE_NOTIFY__

#include <Arduino.h>
#include <WiFi.h>
#include "FS.h"

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
            void * buffer = NULL;
            size_t size = 0;
        } data;

        // image form external storage
        File file;
    } image;
} LINE_Notify_Massage_Option_t;


class ArtronShop_LineNotify {
    private:
        String token = "";
        Client * client = NULL;
 
    public:
        ArtronShop_LineNotify() ;

        void begin(String token, Client * client = NULL) ;
        void setToken(String token) ;
        void setClient(Client * client) ;

        bool send(String massage) ;
        bool send(String massage, LINE_Notify_Massage_Option_t *option) ;

};

extern ArtronShop_LineNotify Notify;

#endif