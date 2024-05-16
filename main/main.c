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
#include "botao.h"

SemaphoreHandle_t mqtt_on_semaphore;

void info_remote()
{
    char json_message[128];
    snprintf(json_message, sizeof(json_message), "{\"water_level\": %.2f, \"pump_status\": \"%s\"}", water_level, is_pump_on ? "on" : "off");
    mqtt_publish_message("rv/info", json_message);
    vTaskDelay(pdMS_TO_TICKS(15000));
    // sensor gas, pressao, temperatura, umidade, level da agua, bomba de agua
    // pub info de tempo em tempo
}

void action_remote(){
    vTaskDelay(pdMS_TO_TICKS(15000));
    // bomba de agua
    // Escutando algum pub e ao receber faz algo localmente, fazer direto no MQTT
}

void set_action(char key){
    char json_message[12];
    int type = 0;
    snprintf(json_message, sizeof(json_message), "{\"type\": %d}", type);

    if(key == "o"){
        mqtt_publish_message("sala11/bomba", "0");
    }
    if(key == "b"){
        mqtt_publish_message("sala11/bomba", "1");
    }
    if(key == 'p'){
        mqtt_publish_message("sala11/valvula", "0");
    }
    if(key == 'v'){
        mqtt_publish_message("sala11/valvula", "1");
    }
    if(key == 'd'){
        mqtt_publish_message("esp32/open_door", "111111111111111111111111111");
    }
    if(key == 'w'){
        mqtt_publish_message("get_drink", "1");
    }
    if(key == '1'){
        type = 1;
        mqtt_publish_message("get_food", json_message);
    }
    if(key == '2'){
        type = 2;
        mqtt_publish_message("get_food", json_message);
    }
    if(key == '3'){
        type = 3;
        mqtt_publish_message("get_food", json_message);
    }
}

void get_info()
{
    ESP_LOGI("TAG_IW", "Time Status: %d", get_time);
    ESP_LOGI("TAG_IW", "Time Status: %d", get_led);
    ESP_LOGI("TAG_IW", "Time Status: %.2f", get_distance);
    ESP_LOGI("TAG_IW", "Time Status: %d", get_agua);
}

void check_network(void *params)
{
    while (true)
    {
        if (xSemaphoreTake(mqtt_on_semaphore, pdMS_TO_TICKS(10000)) == pdTRUE)
        {
            ESP_LOGI(TAG_M, "Sucessfully take semaphore from MQTT");
            xSemaphoreGive(mqtt_on_semaphore);
            register_key_callback(set_action);
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

    mqtt_on_semaphore = xSemaphoreCreateBinary();
    if (mqtt_on_semaphore == NULL)
    {
        ESP_LOGE("MQTT", "Failed to create semaphores");
        return;
    }

    ESP_LOGI(TAG_W, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    mqtt_start();
    vTaskDelay(pdMS_TO_TICKS(100));

    led_task_running = true;
    xTaskCreate(led_task, "Led", 4096, NULL, 1, NULL);

    ESP_LOGI(TAG_WA, "Initializing Water Tank");
    xTaskCreate(water_tank_task, "Water Tank Task", 4096, NULL, 1, NULL);
    ESP_LOGI(TAG_WP, "Initializing Water Pump");
    xTaskCreate(water_pump_task, "Water Pump Task", 4096, NULL, 1, NULL);
    ESP_LOGI(TAG_BME280, "Initializing I2C Bus for BME280 Sensor");
    i2c_master_init();
    ESP_LOGI(TAG_BME280, "Initializing BME280 Sensor");
    xTaskCreate(bme280_task, "BME280 Task", 1024 * 5, NULL, 5, NULL);
    ESP_LOGI(TAG_BME280, "Initializing MQ2 Sensor");
    xTaskCreate(read_mq2_sensor_task, "MQ2 Sensor Task", 4096, NULL, 5, NULL);

    xTaskCreate(&key_task, "Key Task", 4096, NULL, 1, NULL);

    xTaskCreate(&check_network, "Check", 4096, NULL, 1, NULL);
}