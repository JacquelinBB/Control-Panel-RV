#include "esp_log.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "freertos/semphr.h"

#include "wifi.h"
#include "mqtt_cloud.h"
#include "sensor_hcsr04.h"

//SemaphoreHandle_t wifi_on_semaphore;
//SemaphoreHandle_t mqtt_on_semaphore;

/*
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

void mqtt_on(void *params)
{
    char mensagem[50];
    if (xSemaphoreTake(mqtt_on_semaphore, portMAX_DELAY) == pdTRUE)
    {
        while (true)
        {
            sprintf(mensagem, "Hello World");
            mqtt_publish_menssage("esp32/test", mensagem);
            xSemaphoreGive(mqtt_on_semaphore); // Libera o semáforo após o uso
            vTaskDelay(10000 / portTICK_PERIOD_MS);
        }
    }
    else
    {
        ESP_LOGE(TAG_M, "Failed to take semaphore");
    }
}
*/

void test_task(void *params) {
    char mensagem[50];
    if (xSemaphoreTake(mqtt_on_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE) {
        sprintf(mensagem, "Hello World");
        mqtt_publish_menssage("esp32/test", mensagem);
        ESP_LOGI(TAG_M, "Message published");
    } else {
        ESP_LOGE(TAG_M, "MQTT connection unavailable. Waiting for reconnection...");
        // Espera até que a conexão MQTT seja restabelecida ou que ocorra um timeout
        if (xSemaphoreTake(mqtt_on_semaphore, pdMS_TO_TICKS(10000)) == pdTRUE) {
            ESP_LOGI(TAG_M, "MQTT connection reestablished. Resuming publication.");
        } else {
            ESP_LOGE(TAG_M, "Timeout waiting for MQTT connection. Retrying...");
        }
    }
    vTaskDelay(pdMS_TO_TICKS(10000)); // Publica a cada 10 segundos
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

    /*
    wifi_on_semaphore = xSemaphoreCreateBinary();
    mqtt_on_semaphore = xSemaphoreCreateBinary();
    if (wifi_on_semaphore == NULL || mqtt_on_semaphore == NULL) {
        ESP_LOGE("Wifi & MQTT", "Failed to create semaphores");
        return;
    }

    ESP_LOGI(TAG_W, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    mqtt_start();

    xTaskCreate(&test_task, "Test Task", 4096, NULL, 1, NULL);
    */
    //xTaskCreate(&wifi_on, "Conecting to MQTT", 4096, NULL, 1, NULL); NOT
    //xTaskCreate(&mqtt_on, "Conected to MQTT", 4096, NULL, 1, NULL); NOT

    ESP_LOGI(TAG, "Initializing HC-SR04 sensor");
    xTaskCreate(ultrasonic_sensor_task, "Ultrasonic Sensor Task", 4096, NULL, 1, NULL);
}