#ifndef MQTT_CLOUD_H
#define MQTT_CLOUD_H

#define TAG_M "MQTT"

#define BROKER_URL CONFIG_AWS_BROKER_URL

//extern char get_pump_status[10];

extern int get_agua, get_acesso_time, get_porta;
extern float get_cisterna, get_caixa;
extern char get_name[30];
extern bool topico_water, topico_door, topico_open_door;

void mqtt_start();
void mqtt_publish_message(char* topic, char *message);

#endif