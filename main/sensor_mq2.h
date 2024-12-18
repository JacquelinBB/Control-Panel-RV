/*
 * Este arquivo está licenciado sob a Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0).
 * Você pode compartilhar, modificar e usar este trabalho para fins não comerciais, desde que atribua os devidos créditos aos autores.
 * Para mais informações sobre a licença, visite: https://creativecommons.org/licenses/by-nc/4.0/
 */

#ifndef MQ2
#define MQ2

#define TAG_GAS "MQ2"

#define MQ2_SENSOR_PIN ADC1_CHANNEL_6

extern uint32_t adc_value;
extern float voltage, Rs, Ro, ratio, mq2_value;

void setup_adc();
void read_mq2_sensor_task(void *pvParameter);

#endif
