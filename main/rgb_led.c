#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "rgb_led.h"

ledc_channel_config_t ledc_channel[3];
bool led_task_running = false;

void configura_pinos_led()
{
    ledc_timer_config_t ledc_timer_conf;

    ledc_timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_timer_conf.duty_resolution = LEDC_TIMER_8_BIT;
    ledc_timer_conf.timer_num = LEDC_TIMER_0;
    ledc_timer_conf.freq_hz = PWM_FREQ;
    ledc_timer_conf.clk_cfg = LEDC_AUTO_CLK;
    ledc_timer_config(&ledc_timer_conf);

    ledc_channel[0] = (ledc_channel_config_t){
        .gpio_num = RED_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = RED_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0};

    ledc_channel[1] = (ledc_channel_config_t){
        .gpio_num = GREEN_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = GREEN_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0};

    ledc_channel[2] = (ledc_channel_config_t){
        .gpio_num = BLUE_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = BLUE_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0};

    for (int i = 0; i < 3; i++)
    {
        ledc_channel_config(&ledc_channel[i]);
    }
}

void set_rgb_color(uint8_t red, uint8_t green, uint8_t blue)
{
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, RED_CHANNEL, red);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, GREEN_CHANNEL, green);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, BLUE_CHANNEL, blue);

    ledc_update_duty(LEDC_HIGH_SPEED_MODE, RED_CHANNEL);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, GREEN_CHANNEL);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, BLUE_CHANNEL);
}

void led_task(void *params)
{
    configura_pinos_led();
    ledc_fade_func_install(0);

    while (1)
    {
        if (!led_task_running)
        {
            break;
            vTaskDelete(NULL);
        }
        set_rgb_color(255, 0, 255);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void led_task_side(void *params)
{
    configura_pinos_led();

    while (1)
    {
        if (!led_task_running)
        {
            break;
        }
        set_rgb_color(0, 0, 0);
    }
    vTaskDelete(NULL);
}
