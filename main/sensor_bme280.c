#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "sensor_bme280.h"

void i2c_master_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ};
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

s8 bme280_i2c_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt) // Função de escrita no barramento I2C para o sensor BME280
{
    esp_err_t espRc;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);                                                // Início da transação I2C
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true); // Endereço do dispositivo + bit de escrita
    i2c_master_write_byte(cmd, reg_addr, true);                           // Endereço do registro a ser escrito
    i2c_master_write(cmd, reg_data, cnt, true);                           // Dados a serem escritos no registro
    i2c_master_stop(cmd);                                                 // Fim da transação I2C

    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS); // Envio do comando I2C
    s8 result = (espRc == ESP_OK) ? SUCCESS : ERROR;                       // Verificação do resultado da transação
    i2c_cmd_link_delete(cmd);                                              // Liberação do comando I2C

    return result;
}

s8 bme280_i2c_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt) // Função de leitura no barramento I2C para o sensor BME280
{
    esp_err_t espRc;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);                                                // Início da transação I2C
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true); // Endereço do dispositivo + bit de escrita
    i2c_master_write_byte(cmd, reg_addr, true);                           // Endereço do registro a ser lido
    i2c_master_start(cmd);                                                // Início de uma nova transação I2C para leitura
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);  // Endereço do dispositivo + bit de leitura

    if (cnt > 1) // Leitura dos dados do registro
    {
        i2c_master_read(cmd, reg_data, cnt - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, reg_data + cnt - 1, I2C_MASTER_NACK);

    i2c_master_stop(cmd); // Fim da transação I2C

    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS); // Envio do comando I2C
    s8 result = (espRc == ESP_OK) ? SUCCESS : ERROR;                       // Verificação do resultado da transação
    i2c_cmd_link_delete(cmd);                                              // Liberação do comando I2C

    return result;
}

void bme280_delay_msec(u32 msek) // Função de atraso em milissegundos
{
    vTaskDelay(msek / portTICK_PERIOD_MS);
}

void bme280_task(void *params) // Tarefa para ler e publicar os dados do sensor BME280
{
    // BME280 I2C communication structure
    struct bme280_t bme280 = {
        .bus_write = bme280_i2c_write,
        .bus_read = bme280_i2c_read,
        .dev_addr = BME280_I2C_ADDRESS1,
        .delay_msec = bme280_delay_msec};

    s32 com_rslt;
    s32 v_uncomp_pressure_s32;
    s32 v_uncomp_temperature_s32;
    s32 v_uncomp_humidity_s32;

    // Inicialização do sensor BME280
    com_rslt = bme280_init(&bme280);
    if (com_rslt == SUCCESS)
    {
        ESP_LOGI(TAG_BME280, "BME280 initialization successful");
    }
    else
    {
        ESP_LOGE(TAG_BME280, "BME280 initialization failed. Error code: %d", com_rslt);
        vTaskDelete(NULL);
        return;
    }

    // Configuração do sensor BME280
    com_rslt += bme280_set_oversamp_pressure(BME280_OVERSAMP_16X);
    com_rslt += bme280_set_oversamp_temperature(BME280_OVERSAMP_2X);
    com_rslt += bme280_set_oversamp_humidity(BME280_OVERSAMP_1X);
    com_rslt += bme280_set_standby_durn(BME280_STANDBY_TIME_1_MS);
    com_rslt += bme280_set_filter(BME280_FILTER_COEFF_16);
    com_rslt += bme280_set_power_mode(BME280_NORMAL_MODE);

    if (com_rslt != SUCCESS)
    {
        ESP_LOGE(TAG_BME280, "Failed to configure BME280. Error code: %d", com_rslt);
        vTaskDelete(NULL);
        return;
    }

    while (true)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);

        // Leitura dos dados do sensor BME280
        com_rslt = bme280_read_uncomp_pressure_temperature_humidity(
            &v_uncomp_pressure_s32, &v_uncomp_temperature_s32, &v_uncomp_humidity_s32);

        // Compensação e formatação dos dados do sensor
        double temp = bme280_compensate_temperature_double(v_uncomp_temperature_s32);
        double press = bme280_compensate_pressure_double(v_uncomp_pressure_s32) / 100; // Pa -> hPa
        double hum = bme280_compensate_humidity_double(v_uncomp_humidity_s32);

        // Impressão dos dados do sensor
        if (com_rslt == SUCCESS)
        {
            printf("Temperature: %.2f°C, Pressure: %.2f hPa, Humidity: %.2f%%\n", temp, press, hum);
        }
        else
        {
            ESP_LOGE(TAG_BME280, "measure error. code: %d", com_rslt);
        }
    }
}