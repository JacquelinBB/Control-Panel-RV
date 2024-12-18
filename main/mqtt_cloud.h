/*
 * Este arquivo está licenciado sob a Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0).
 * Você pode compartilhar, modificar e usar este trabalho para fins não comerciais, desde que atribua os devidos créditos aos autores.
 * Para mais informações sobre a licença, visite: https://creativecommons.org/licenses/by-nc/4.0/
 */

#ifndef MQTT_CLOUD_H
#define MQTT_CLOUD_H

#define TAG_M "MQTT"

#define BROKER_URL CONFIG_AWS_BROKER_URL

//extern char get_pump_status[10];

void mqtt_start();
void mqtt_stop();
void mqtt_publish_message(char* topic, char *message);

#endif
