#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "water_pump.h"

void init_config_water_pump() {
    gpio_config_t config; // Estrutura de configuração do pino GPIO

    // Configura o pino GPIO do relé como saída
    config.intr_type = GPIO_INTR_DISABLE; // Desabilita interrupções para este pino
    config.mode = GPIO_MODE_OUTPUT; // Configura o modo do pino como saída
    config.pin_bit_mask = (1ULL << RELAY_PIN); // Configura o bit do pino
    config.pull_down_en = GPIO_PULLDOWN_DISABLE; // Desabilita resistor de pull-down
    config.pull_up_en = GPIO_PULLUP_DISABLE; // Desabilita resistor de pull-up
    gpio_config(&config); // Aplica a configuração ao pino

    gpio_set_level(RELAY_PIN, 0); // Desliga o relé inicialmente
}
