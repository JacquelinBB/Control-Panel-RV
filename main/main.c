/*
 * Este arquivo está licenciado sob a Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0).
 * Você pode compartilhar, modificar e usar este trabalho para fins não comerciais, desde que atribua os devidos créditos aos autores.
 * Para mais informações sobre a licença, visite: https://creativecommons.org/licenses/by-nc/4.0/
 */

#include "esp_log.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "freertos/semphr.h"
#include "wifi.h"
#include "mqtt_cloud.h"
#include "sensor_hcsr04.h"
#include "water_pump.h"
#include "rgb_led.h"
#include "sensor_bme280.h"
#include "sensor_mq2.h"
#include "screen.h"
#include "ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

void app_main()
{
    ESP_LOGI("Info", "[APP] Startup..");
    ESP_LOGI("Info", "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI("Info", "[APP] IDF version: %s", esp_get_idf_version());
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("transport", ESP_LOG_VERBOSE);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Inicializando NimBLE...");
    ble_security_init();
    nimble_port_init();         
    ble_svc_gap_device_name_set("BLE-Server");
    ble_svc_gap_init();               
    ble_svc_gatt_init();             
    ble_gatts_count_cfg(gatt_svr_svcs);      
    ble_gatts_add_svcs(gatt_svr_svcs);  
    ble_hs_cfg.sync_cb = ble_app_on_sync;  
    nimble_port_freertos_init(ble_host_task);        
    vTaskDelay(pdMS_TO_TICKS(100));

    //xTaskCreate(led_task, "Led", 4096, NULL, 1, NULL);

    ESP_LOGI(TAG_WA, "Initializing Water Tank");
    xTaskCreate(water_tank_task, "Water Tank Task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG_WP, "Initializing Water Pump");
    init_config_water_pump();
    ESP_LOGI(TAG_BME280, "Initializing I2C Bus for BME280 Sensor");
    i2c_master_init();
    ESP_LOGI(TAG_BME280, "Initializing BME280 Sensor");
    xTaskCreate(bme280_task, "BME280 Task", 1024 * 5, NULL, 5, NULL);
    ESP_LOGI(TAG_BME280, "Initializing MQ2 Sensor");
    xTaskCreate(read_mq2_sensor_task, "MQ2 Sensor Task", 4096, NULL, 5, NULL);

    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
}
