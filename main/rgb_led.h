#ifndef LED_RGB
#define LED_RGB

#define AZUL_PIN GPIO_NUM_12

void configuraPinosLed();
void led_task(void *params);

#endif