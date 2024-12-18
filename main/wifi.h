/*
 * Este arquivo está licenciado sob a Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0).
 * Você pode compartilhar, modificar e usar este trabalho para fins não comerciais, desde que atribua os devidos créditos aos autores.
 * Para mais informações sobre a licença, visite: https://creativecommons.org/licenses/by-nc/4.0/
 */

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
void wifi_stop();

#endif
