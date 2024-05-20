#ifndef MQTT_CLOUD_H
#define MQTT_CLOUD_H

#define TAG_M "MQTT"

#define BROKER_URL CONFIG_AWS_BROKER_URL

extern SemaphoreHandle_t mqtt_on_semaphore;

//extern char get_pump_status[10];

extern int get_time, get_led, get_agua, get_fluxo_time, get_fluxo, get_acesso_time, get_porta;
extern float get_distance, get_cisterna, get_caixa;
extern char get_name[30], get_food1[5], get_food2[5], get_food3[5], get_drink[5];
extern bool topico_water, topico_fluxo, topico_door, topico_open_door, topico_food, topico_drink;

void mqtt_start();
void mqtt_publish_message(char* topic, char *message);

#endif