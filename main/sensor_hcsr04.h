/*
 * Este arquivo está licenciado sob a Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0).
 * Você pode compartilhar, modificar e usar este trabalho para fins não comerciais, desde que atribua os devidos créditos aos autores.
 * Para mais informações sobre a licença, visite: https://creativecommons.org/licenses/by-nc/4.0/
 */

#ifndef HC_SR04
#define HC_SR04

#define TRIGGER_GPIO GPIO_NUM_33
#define ECHO_GPIO GPIO_NUM_26
#define TANK_HEIGHT_CM 14 // Altura do tanque em centímetro é de 14 ou 17

#define GPIO_OUTPUT_PIN_SEL (1ULL << TRIGGER_GPIO)
#define GPIO_INPUT_PIN_SEL (1ULL << ECHO_GPIO)

#define TAG_WA "WATER_LEVEL"

extern float water_level;

void init_config_water_tank();
float calcular_nivel_agua(float distancia);
void water_tank_task(void *params);

#endif
