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

void set_action(){
    char json_message[12];
    int type = 0;
    snprintf(json_message, sizeof(json_message), "{\"type\": %d}", type);

    mqtt_publish_message("sala11/bomba", "0");
    mqtt_publish_message("sala11/bomba", "1");
    mqtt_publish_message("sala11/valvula", "0");
    mqtt_publish_message("sala11/valvula", "1");
    mqtt_publish_message("esp32/open_door", "111111111111111111111111111");
    mqtt_publish_message("get_drink", "1");
    type = 1;
    mqtt_publish_message("get_food", json_message);
    type = 2;
    mqtt_publish_message("get_food", json_message);
    type = 3;
    mqtt_publish_message("get_food", json_message);

    vTaskDelay(pdMS_TO_TICKS(10000));
}

void get_info()
{
    if(topico_water == true)
    {
        topico_water = false;
        ESP_LOGI("TAG_IW", "Status do time: %d", get_time);
        ESP_LOGI("TAG_IW", "Status do led: %d", get_led);
        ESP_LOGI("TAG_IW", "Status da Distância: %.2f cm", get_distance);
        ESP_LOGI("TAG_IW", "Status do Nível de água: %d", get_agua);
        ESP_LOGI("TAG_IW", "Status do Volume da cisterna: %.2f", get_cisterna);
        ESP_LOGI("TAG_IW", "Status do Volume da caixa: %.2f", get_caixa);
    }
    else if(topico_fluxo == true)
    {
        topico_fluxo = false;
        ESP_LOGI("TAG_IF", "Status do Tempo do Fluxo: %d", get_fluxo_time);
        ESP_LOGI("TAG_IF", "Status do Fluxo de água: %d", get_fluxo);
    }
    else if(topico_food == true)
    { // x y z
        topico_food = false;
        ESP_LOGI("TAG_F", "Status da quantidade de comida no tipo 1: %s", get_food1);
        ESP_LOGI("TAG_F", "Status da quantidade de comida no tipo 2: %s", get_food2);
        ESP_LOGI("TAG_F", "Status da quantidade de comida no tipo 3: %s", get_food3);
    }
    else if(topico_drink == true)
    {
        topico_drink = false;
        ESP_LOGI("TAG_D", "Status da quantidade de líquido disponivel: %s", get_drink);
    }
    else if(topico_door == true)
    {
        topico_door = false;
        ESP_LOGI("TAG_P", "Status da porta: %d", get_porta);
        ESP_LOGI("TAG_P", "Último acesso registrado: %d", get_acesso_time); // Milisegundos
    }
    else if(topico_open_door == true)
    {
        topico_open_door = false;
        ESP_LOGI("TAG_OD", "Último acesso por: %s", get_name); // is_valid: 0 ou 1 | rfid | name: string)
    }
    // "esp32/pub_dgp" - Código do rfid para controle de acesso (string) = "sensor_a0:" "79 88 0A 00"
    // "esp32/pinPad" - "sensor_a0: (int) - Senha que foi digitada no teclado
}

void check_network(void *params)
{
    while (true)
    {
        if (xSemaphoreTake(mqtt_on_semaphore, pdMS_TO_TICKS(10000)) == pdTRUE)
        {
            xSemaphoreGive(mqtt_on_semaphore);
            //set_action();
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

    //xTaskCreate(led_task, "Led", 4096, NULL, 1, NULL);

    //ESP_LOGI(TAG_WA, "Initializing Water Tank");
    //xTaskCreate(water_tank_task, "Water Tank Task", 4096, NULL, 1, NULL);
    //pump_task_running = true;
    //ESP_LOGI(TAG_WP, "Initializing Water Pump");
    //xTaskCreate(water_pump_task, "Water Pump Task", 4096, NULL, 1, NULL);
    //ESP_LOGI(TAG_BME280, "Initializing I2C Bus for BME280 Sensor");
    //i2c_master_init();
    //ESP_LOGI(TAG_BME280, "Initializing BME280 Sensor");
    //xTaskCreate(bme280_task, "BME280 Task", 1024 * 5, NULL, 5, NULL);
    //ESP_LOGI(TAG_BME280, "Initializing MQ2 Sensor");
    //xTaskCreate(read_mq2_sensor_task, "MQ2 Sensor Task", 4096, NULL, 5, NULL);

    xTaskCreate(&check_network, "Check", 4096, NULL, 1, NULL);
}