#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "botao.h"

#define UART_NUM UART_NUM_1
#define BUF_SIZE (1024)

static void (*key_callback)(char key);

void uart_init()
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_NUM, &uart_config);
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
}

char read_key()
{
    uint8_t data;
    if (uart_read_bytes(UART_NUM, &data, 1, pdMS_TO_TICKS(10)) > 0)
    {
        return (char)data;
    }
    return '\0';
}

void register_key_callback(void (*callback)(char key))
{
    key_callback = callback;
}

void key_task(void *params)
{
    uart_init();
    
    while (1)
    {
        char key = read_key();
        if (key != '\0' && key_callback)
        {
            key_callback(key);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
