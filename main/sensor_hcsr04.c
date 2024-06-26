#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "sensor_hcsr04.h"

float water_level = 0.0;

void init_config_water_tank()
{
    gpio_config_t config;
    config.intr_type = GPIO_INTR_DISABLE;      // Desabilita interrupções geradas pelo pino
    config.mode = GPIO_MODE_OUTPUT;            // Define o pino como saída, para enviar o pulso de trigger
    config.pin_bit_mask = GPIO_OUTPUT_PIN_SEL; // Define o pino que será configurado
    config.pull_down_en = 0;                   // Desabilita resistor de pull-down do pino
    config.pull_up_en = 0;                     // Desabilita resistor de pull-up do pino
    gpio_config(&config);                      // Aplica a configuração ao pino

    config.mode = GPIO_MODE_INPUT;            // Define o pino como entrada, para receber o pulso do ECHO
    config.pin_bit_mask = GPIO_INPUT_PIN_SEL; // Define o pino que será configurado
    gpio_config(&config);                     // Aplica a configuração ao pino
}

float calcular_nivel_agua(float distancia)
{
    // Calcular a altura da água em relação à altura total do tanque
    float nivel_agua = ((TANK_HEIGHT_CM - distancia) / TANK_HEIGHT_CM) * 100;

    // Limitar o intervalo de 0 a 100%
    if (nivel_agua < 0)
    {
        nivel_agua = 0;
    }
    else if (nivel_agua > 100)
    {
        nivel_agua = 100;
    }

    return nivel_agua;
}

void water_tank_task(void *params)
{
    init_config_water_tank();
    uint64_t start, end;

    while (1)
    {
        // Enviando um 10us pulso para o pino Trigger
        gpio_set_level(TRIGGER_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(10)); // Atraso de 10 microssegundos
        gpio_set_level(TRIGGER_GPIO, 0);

        // Esperando o pino ECHO ficar alto
        while (gpio_get_level(ECHO_GPIO) == 0)
            ;
        start = esp_timer_get_time();

        // Esperando o pino ECHO ficar baixo novamente
        while (gpio_get_level(ECHO_GPIO) == 1)
            ;
        end = esp_timer_get_time();

        // Calcular a distância em cm
        uint32_t duration = end - start;
        float distance = duration * 0.0343 / 2;

        // Calcular a porcentagem de água
        water_level = calcular_nivel_agua(distance);
        vTaskDelay(pdMS_TO_TICKS(10));
        //printf("Water level: %.2f%%\n", water_level);

        vTaskDelay(pdMS_TO_TICKS(1000)); // Aguardar 1 segundo
    }
}