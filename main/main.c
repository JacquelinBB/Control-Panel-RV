#include "esp_log.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "freertos/semphr.h"

#include "wifi.h"
#include "mqtt_cloud.h"

/*
SemaphoreHandle_t wifi_on_semaphore;

void wifi_on(void *params)
{
    while (true)
    {
        if (xSemaphoreTake(wifi_on_semaphore, portMAX_DELAY) == pdTRUE)
        {
            mqtt_start();
            xSemaphoreGive(wifi_on_semaphore); // Libera o semáforo após o uso
        }
        else
        {
            ESP_LOGE(TAG_M, "Failed to take semaphore");
        }
    }
}
*/

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

    ESP_LOGI(TAG_W, "ESP_WIFI_MODE_STA");
    /*
    wifi_on_semaphore = xSemaphoreCreateBinary();
    if (wifi_on_semaphore == NULL)
    {
        ESP_LOGE(TAG_M, "Failed to create semaphore");
        return;
    }
    */
    wifi_init_sta();
    mqtt_start();
    // xTaskCreate(&wifi_on, "Conecting to MQTT", 4096, NULL, 1, NULL);
}