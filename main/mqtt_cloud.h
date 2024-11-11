#ifndef MQTT_CLOUD_H
#define MQTT_CLOUD_H

#define TAG_M "MQTT"

#define BROKER_URL CONFIG_AWS_BROKER_URL

//extern char get_pump_status[10];

void mqtt_start();
void mqtt_stop();
void mqtt_publish_message(char* topic, char *message);

#endif