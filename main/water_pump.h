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