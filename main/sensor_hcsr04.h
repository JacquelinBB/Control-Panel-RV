#ifndef HC_SR04
#define HC_SR04

#define TRIGGER_GPIO GPIO_NUM_27
#define ECHO_GPIO GPIO_NUM_26
#define TANK_HEIGHT_CM 14 // Altura do tanque em centímetro é de 14 ou 17

#define GPIO_OUTPUT_PIN_SEL (1ULL << TRIGGER_GPIO)
#define GPIO_INPUT_PIN_SEL (1ULL << ECHO_GPIO)

#define TAG_WA "WATER_LEVEL"

void configuraPinos();
float calcular_nivel_agua(float distancia);
void ultrasonic_task(void *params);

#endif