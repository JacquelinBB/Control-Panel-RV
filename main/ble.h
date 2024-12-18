/*
 * Este arquivo está licenciado sob a Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0).
 * Você pode compartilhar, modificar e usar este trabalho para fins não comerciais, desde que atribua os devidos créditos aos autores.
 * Para mais informações sobre a licença, visite: https://creativecommons.org/licenses/by-nc/4.0/
 */

#ifndef BLE_H
#define BLE_H

#include "host/ble_gatt.h"
#define TAG_B "NimBLE_BLE"

#define SENSOR_SERVICE_UUID       0x1234
#define WATER_LEVEL_UUID          0x1235
#define ENVIRONMENTAL_UUID        0x1236
#define ACTUATOR_UUID             0x1237

extern const struct ble_gatt_svc_def gatt_svr_svcs[];
extern bool is_extern_activate;

void ble_app_on_sync(void);
void ble_app_advertise(void);
void ble_security_init(void);
void ble_host_task(void *param);
void info_mqtt();

#endif
