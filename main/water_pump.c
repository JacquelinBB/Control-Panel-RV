#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "water_pump.h"

bool is_pump_on = false;
bool is_button_press = false;
bool pump_task_running = false;

void init_config_water_pump() {
    gpio_config_t config; // Estrutura de configuração do pino GPIO
    
    // Configuração do pino do botão
    config.pin_bit_mask = (1 << BOTAO_PIN);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_ENABLE; // Ativa o resistor de pull-up interno para evitar flutuações no pino do botão
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&config);

    // Configura o pino GPIO do relé como saída
    config.intr_type = GPIO_INTR_DISABLE; // Desabilita interrupções para este pino
    config.mode = GPIO_MODE_OUTPUT; // Configura o modo do pino como saída
    config.pin_bit_mask = (1ULL << RELAY_PIN); // Configura o bit do pino
    config.pull_down_en = GPIO_PULLDOWN_DISABLE; // Desabilita resistor de pull-down
    config.pull_up_en = GPIO_PULLUP_DISABLE; // Desabilita resistor de pull-up
    gpio_config(&config); // Aplica a configuração ao pino

    gpio_set_level(RELAY_PIN, 0); // Desliga o relé inicialmente
}

void water_pump_task() {
    init_config_water_pump();

    while (true)
    {
        /*
        if (!pump_task_running)
        {
            break;
            vTaskDelete(NULL);
        }
        */
        
        if (gpio_get_level(BOTAO_PIN) == 0) // Verifica se o botão foi pressionado (nível alto devido à configuração do pull-up)
        {
            is_button_press = true;
        }
        else{
            if (is_button_press) {
                if (is_pump_on) {
                    gpio_set_level(RELAY_PIN, 0);
                } else {
                    gpio_set_level(RELAY_PIN, 1);
                }
                printf("Water pump is %s\n", is_pump_on ? "off" : "on");
                is_pump_on = !is_pump_on;
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
            is_button_press = false;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
