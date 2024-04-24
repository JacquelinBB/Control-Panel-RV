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

SemaphoreHandle_t mqtt_on_semaphore;

void set_info()
{
    char json_message[128];
    snprintf(json_message, sizeof(json_message), "{\"water_level\": %.2f, \"pump_status\": \"%s\"}", water_level, is_pump_on ? "on" : "off");
    mqtt_publish_message("rv/info", json_message);
    vTaskDelay(pdMS_TO_TICKS(15000));
} // set_action

void get_info()
{
    ESP_LOGI(TAG_WA, "Water Level: %.2f", get_water_level);
    ESP_LOGI(TAG_WP, "Pump Status: %s", get_pump_status);
}

void check_network(void *params)
{
    while (true)
    {
        if (xSemaphoreTake(mqtt_on_semaphore, pdMS_TO_TICKS(10000)) == pdTRUE)
        {
            ESP_LOGI(TAG_M, "Sucessfully take semaphore from MQTT");
            xSemaphoreGive(mqtt_on_semaphore);
            set_info();
            get_info();
        }
        else
        {
            ESP_LOGE(TAG_W, "Failed to take semaphore from MQTT, because MQTT connection is unavailable");
        }
    }
}

void app_main()
{
    ESP_LOGI(TAG_W, "[APP] Startup..");
    ESP_LOGI(TAG_W, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG_W, "[APP] IDF version: %s", esp_get_idf_version());
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

    mqtt_on_semaphore = xSemaphoreCreateBinary();
    if (mqtt_on_semaphore == NULL)
    {
        ESP_LOGE("MQTT", "Failed to create semaphores");
        return;
    }

    ESP_LOGI(TAG_W, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    mqtt_start();
    vTaskDelay(pdMS_TO_TICKS(5000));

    ESP_LOGI(TAG_WA, "Initializing Water Tank");
    xTaskCreate(water_tank_task, "Water Tank Task", 4096, NULL, 1, NULL);
    ESP_LOGI(TAG_WP, "Initializing Water Pump");
    xTaskCreate(water_pump_task, "Water Pump Task", 4096, NULL, 1, NULL);
    ESP_LOGI(TAG_BME280, "Initializing I2C Bus for BME280 Sensor");
    i2c_master_init();
    ESP_LOGI(TAG_BME280, "Initializing BME280 Sensor");
    xTaskCreate(bme280_task, "BME280 Task", 1024 * 5, NULL, 5, NULL);

    // xTaskCreate(&check_network, "Check", 4096, NULL, 1, NULL);
}