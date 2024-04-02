#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Definições dos pinos GPIO para cada LED e botão
#define AZUL_PIN GPIO_NUM_12
#define BOTAO_PIN GPIO_NUM_14

// Função para configurar os pinos GPIO
void configuraPinos()
{
    gpio_config_t config;

    // Configuração do pino do LED
    config.pin_bit_mask = (1ULL << AZUL_PIN);
    config.mode = GPIO_MODE_OUTPUT;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&config);

    // Configuração do pino do botão
    config.pin_bit_mask = (1 << BOTAO_PIN);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_ENABLE; // Ativa o resistor de pull-up interno para evitar flutuações no pino do botão
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&config);
}

void led_button_task(void *params)
{
    // Configura os pinos GPIO
    configuraPinos();

    while (1)
    {
        ESP_LOGI("NOT BUTTON", "Aguardando botão...");
        // Verifica se o botão foi pressionado (nível alto devido à configuração do pull-up)
        while (gpio_get_level(BOTAO_PIN) == 1)
        {
            // Liga o LED
            ESP_LOGI("BUTTON", "Botão pressionado");
            vTaskDelay(10 / portTICK_PERIOD_MS);
            gpio_set_level(AZUL_PIN, 1);
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        gpio_set_level(AZUL_PIN, 0);

        // Pequeno atraso para liberar o processador
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
