/*
 * Este arquivo está licenciado sob a Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0).
 * Você pode compartilhar, modificar e usar este trabalho para fins não comerciais, desde que atribua os devidos créditos aos autores.
 * Para mais informações sobre a licença, visite: https://creativecommons.org/licenses/by-nc/4.0/
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "water_pump.h"
#include <time.h>

time_t pump_start_time;
float total_energy_consumed = 0.0;

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

void start_pump() {
    gpio_set_level(RELAY_PIN, 1);
    pump_start_time = time(NULL); // Registra o horário de início
}

void stop_pump() {
    gpio_set_level(RELAY_PIN, 0);

    // Calcula o tempo de operação da bomba em segundos
    time_t pump_end_time = time(NULL);
    double elapsed_time= (double)(pump_end_time - pump_start_time); 
    total_energy_consumed += (PUMP_POWER_WATTS * elapsed_time) / 3600.0; // Calcula o consumo de energia em Wh

    ESP_LOGI("Bomba", "Consumo total acumulado: %.2f Wh", total_energy_consumed);
}
