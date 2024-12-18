/*
 * Este arquivo está licenciado sob a Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0).
 * Você pode compartilhar, modificar e usar este trabalho para fins não comerciais, desde que atribua os devidos créditos aos autores.
 * Para mais informações sobre a licença, visite: https://creativecommons.org/licenses/by-nc/4.0/
 */

#ifndef WATER_PUMP
#define WATER_PUMP

#define TAG_WP "WATER PUMP"

#define RELAY_PIN GPIO_NUM_25
#define PUMP_POWER_WATTS 1.5

extern float total_energy_consumed;

void init_config_water_pump();
void start_pump();
void stop_pump();

#endif
