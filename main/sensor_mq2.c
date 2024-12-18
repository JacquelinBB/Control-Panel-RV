/*
 * Este arquivo está licenciado sob a Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0).
 * Você pode compartilhar, modificar e usar este trabalho para fins não comerciais, desde que atribua os devidos créditos aos autores.
 * Para mais informações sobre a licença, visite: https://creativecommons.org/licenses/by-nc/4.0/
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "sensor_mq2.h"

uint32_t adc_value;
float voltage, Rs, Ro, ratio, mq2_value;

void setup_adc()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(MQ2_SENSOR_PIN, ADC_ATTEN_DB_0);
}

float calculate_PPM(float ratio)
{
    // Os pontos de dados do gráfico de características de sensibilidade
    float concentrations[] = {100, 200, 500, 1000, 2000}; // Concentrações de gás em PPM
    float resistances[] = {0.1, 0.2, 0.5, 1.0, 2.0};      // Razões Rs/Ro

    // Realiza interpolação linear para encontrar a concentração correspondente
    int i;
    float PPM = 0;
    for (i = 0; i < 4; ++i)
    {
        if (ratio >= resistances[i] && ratio <= resistances[i + 1])
        {
            // Fórmula de interpolação linear: y = y1 + (x - x1) * ((y2 - y1) / (x2 - x1))
            PPM = concentrations[i] + (ratio - resistances[i]) * ((concentrations[i + 1] - concentrations[i]) / (resistances[i + 1] - resistances[i]));
            break;
        }
    }
    // Para valores fora do intervalo, use a extrapolação
    if (i == 4)
    {
        // Fórmula de extrapolação linear para simplicidade
        PPM = concentrations[4] + (ratio - resistances[4]) * ((concentrations[4] - concentrations[3]) / (resistances[4] - resistances[3]));
    }

    return PPM;
}

void read_mq2_sensor_task(void *pvParameter)
{
    setup_adc();

    while (1)
    {
        // Leia o valor ADC do sensor MQ-2
        adc_value = adc1_get_raw(MQ2_SENSOR_PIN);

        // Converta o valor ADC para tensão
        voltage = ((float)adc_value / 4095.0) * 3.3; // 3.3V é a referência ADC

        Ro = 1000; // Valor típico de Ro em ar fresco
        Rs = (float)adc_value / Ro;

        // Calcule o valor PPM usando a função de interpolação
        mq2_value = calculate_PPM(Rs);

        //printf("ADC Value: %lu, Voltage: %.2fV, PPM: %.2f\n", (unsigned long)adc_value, voltage, mq2_value);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
