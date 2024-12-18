/*
 * Este arquivo está licenciado sob a Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0).
 * Você pode compartilhar, modificar e usar este trabalho para fins não comerciais, desde que atribua os devidos créditos aos autores.
 * Para mais informações sobre a licença, visite: https://creativecommons.org/licenses/by-nc/4.0/
 */

#ifndef BME280
#define BME280

#include "bme280.h"

#define TAG_BME280 "BME280"

#define SDA_PIN GPIO_NUM_21
#define SCL_PIN GPIO_NUM_22
#define I2C_MASTER_FREQ_HZ 1000000
#define I2C_MASTER_ACK 0
#define I2C_MASTER_NACK 1

extern double temp, press, hum;

void i2c_master_init();
s8 bme280_i2c_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
s8 bme280_i2c_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
void bme280_delay_msec(u32 msek);
void bme280_task(void *params);

#endif
