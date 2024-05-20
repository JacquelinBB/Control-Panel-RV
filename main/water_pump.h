#ifndef WATER_PUMP
#define WATER_PUMP

#define TAG_WP "WATER PUMP"

#define BOTAO_PIN GPIO_NUM_14
#define RELAY_PIN GPIO_NUM_25

extern bool is_pump_on;
extern bool is_button_press;
extern bool pump_task_running;

void init_config_water_pump();
void water_pump_task();

#endif