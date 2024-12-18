/*
 * Este arquivo está licenciado sob a Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0).
 * Você pode compartilhar, modificar e usar este trabalho para fins não comerciais, desde que atribua os devidos créditos aos autores.
 * Para mais informações sobre a licença, visite: https://creativecommons.org/licenses/by-nc/4.0/
 */

#ifndef LED_RGB
#define LED_RGB

#define RED_PIN GPIO_NUM_16
#define GREEN_PIN GPIO_NUM_17
#define BLUE_PIN GPIO_NUM_33

#define RED_CHANNEL LEDC_CHANNEL_0
#define GREEN_CHANNEL LEDC_CHANNEL_1
#define BLUE_CHANNEL LEDC_CHANNEL_2

#define PWM_FREQ 5000

void configura_pinos_led();
void set_rgb_color(uint8_t red, uint8_t green, uint8_t blue);
void led_task(void *params);

#endif
