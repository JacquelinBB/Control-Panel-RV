#ifndef MQ2
#define MQ2

#define TAG_GAS "MQ2"

#define MQ2_SENSOR_PIN ADC1_CHANNEL_6

extern uint32_t adc_value;
extern float voltage, Rs, Ro, ratio, mq2_value;

void setup_adc();
void read_mq2_sensor_task(void *pvParameter);

#endif