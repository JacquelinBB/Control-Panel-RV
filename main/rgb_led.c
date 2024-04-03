#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "rgb_led.h"

void configuraPinosLed()
{
    gpio_config_t config;

    // Configuração do pino do LED
    config.pin_bit_mask = (1ULL << AZUL_PIN);
    config.mode = GPIO_MODE_OUTPUT;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&config);
}

void led_task(void *params)
{
    configuraPinosLed();

    while (1)
    {
        gpio_set_level(AZUL_PIN, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(AZUL_PIN, 1);      
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
