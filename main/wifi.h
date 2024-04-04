#ifndef WIFI_H
#define WIFI_H

#include "esp_event.h"

#define TAG_W "Wifi"

#define WIFI_SSID CONFIG_ESP_WIFI_SSID             // Nome da rede
#define WIFI_PASS CONFIG_ESP_WIFI_PASSWORD         // Senha da rede
#define ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY // Tentativas da ESP se conectar a rede

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

void wifi_init_sta(void);

#endif
