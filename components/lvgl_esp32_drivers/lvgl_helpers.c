/**
 * @file lvgl_helpers.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "sdkconfig.h"
#include "lvgl_helpers.h"
#include "esp_log.h"

#include "lvgl_tft/disp_spi.h"
#include "lvgl_touch/tp_spi.h"

#include "lvgl_spi_conf.h"

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "src/lv_core/lv_refr.h"
#else
#include "lvgl/src/lv_core/lv_refr.h"
#endif

/*********************
 *      DEFINES
 *********************/

 #define TAG "lvgl_helpers"

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/* Interface and driver initialization */
void lvgl_driver_init(void)
{
    ESP_LOGI(TAG, "Display hor size: %d, ver size: %d", LV_HOR_RES_MAX, LV_VER_RES_MAX);
    ESP_LOGI(TAG, "Display buffer size: %d", DISP_BUF_SIZE);

#if defined (CONFIG_LV_TFT_DISPLAY_CONTROLLER_FT81X)
    ESP_LOGI(TAG, "Initializing SPI master for FT81X");

    lvgl_spi_driver_init(TFT_SPI_HOST,
        DISP_SPI_MISO, DISP_SPI_MOSI, DISP_SPI_CLK,
        SPI_BUS_MAX_TRANSFER_SZ, 1,
        DISP_SPI_IO2, DISP_SPI_IO3);
    
    disp_spi_add_device(TFT_SPI_HOST);
    disp_driver_init();

#if defined (CONFIG_LV_TOUCH_CONTROLLER_FT81X)
    touch_driver_init();
#endif

    return;
#endif

#if defined (SHARED_SPI_BUS)
    ESP_LOGI(TAG, "Initializing shared SPI master");

    lvgl_spi_driver_init(TFT_SPI_HOST,
        TP_SPI_MISO, DISP_SPI_MOSI, DISP_SPI_CLK,
        SPI_BUS_MAX_TRANSFER_SZ, 1,
        -1, -1);
    
    disp_spi_add_device(TFT_SPI_HOST);
    tp_spi_add_device(TOUCH_SPI_HOST);
    
    disp_driver_init();
    touch_driver_init();

    return;
#endif

/* Display controller initialization */
#if defined CONFIG_LV_TFT_DISPLAY_PROTOCOL_SPI
    ESP_LOGI(TAG, "Initializing SPI master for display");
    
    lvgl_spi_driver_init(TFT_SPI_HOST,
        DISP_SPI_MISO, DISP_SPI_MOSI, DISP_SPI_CLK,
        SPI_BUS_MAX_TRANSFER_SZ, 1,
        DISP_SPI_IO2, DISP_SPI_IO3);
    
    disp_spi_add_device(TFT_SPI_HOST);
    
    disp_driver_init();
#else
#error "No protocol defined for display controller"
#endif

/* Touch controller initialization */
#if CONFIG_LV_TOUCH_CONTROLLER != TOUCH_CONTROLLER_NONE
    #if defined (CONFIG_LV_TOUCH_DRIVER_PROTOCOL_SPI)
        ESP_LOGI(TAG, "Initializing SPI master for touch");
        
        lvgl_spi_driver_init(TOUCH_SPI_HOST,
            TP_SPI_MISO, TP_SPI_MOSI, TP_SPI_CLK,
            0 /* Defaults to 4094 */, 2,
            -1, -1);
        
        tp_spi_add_device(TOUCH_SPI_HOST);
        
        touch_driver_init();
    #elif defined (CONFIG_LV_TOUCH_DRIVER_PROTOCOL_I2C)
        ESP_LOGI(TAG, "Initializing I2C master for touch");
        
        lvgl_i2c_driver_init(TOUCH_I2C_PORT,
            TOUCH_I2C_SDA, TOUCH_I2C_SCL,
            TOUCH_I2C_SPEED_HZ);
        
        touch_driver_init();
    #elif defined (CONFIG_LV_TOUCH_DRIVER_ADC)
        touch_driver_init();
    #elif defined (CONFIG_LV_TOUCH_DRIVER_DISPLAY)
        touch_driver_init();
    #else
    #error "No protocol defined for touch controller"
    #endif
#else
#endif
}


/* Initialize spi bus master */
bool lvgl_spi_driver_init(int host,
    int miso_pin, int mosi_pin, int sclk_pin,
    int max_transfer_sz,
    int dma_channel,
    int quadwp_pin, int quadhd_pin)
{
#if defined (CONFIG_IDF_TARGET_ESP32)
    assert((SPI_HOST <= host) && (VSPI_HOST >= host));
    const char *spi_names[] = {
        "SPI_HOST", "HSPI_HOST", "VSPI_HOST"
    };
#elif defined (CONFIG_IDF_TARGET_ESP32S2)
    assert((SPI_HOST <= host) && (HSPI_HOST >= host));
    const char *spi_names[] = {
        "SPI_HOST", "", ""
    };
#endif

    ESP_LOGI(TAG, "Configuring SPI host %s (%d)", spi_names[host], host);
    ESP_LOGI(TAG, "MISO pin: %d, MOSI pin: %d, SCLK pin: %d, IO2/WP pin: %d, IO3/HD pin: %d",
        miso_pin, mosi_pin, sclk_pin, quadwp_pin, quadhd_pin);

    ESP_LOGI(TAG, "Max transfer size: %d (bytes)", max_transfer_sz);

    spi_bus_config_t buscfg = {
        .miso_io_num = miso_pin,
	.mosi_io_num = mosi_pin,
	.sclk_io_num = sclk_pin,
	.quadwp_io_num = quadwp_pin,
	.quadhd_io_num = quadhd_pin,
        .max_transfer_sz = max_transfer_sz
    };

    ESP_LOGI(TAG, "Initializing SPI bus...");
    esp_err_t ret = spi_bus_initialize(host, &buscfg, dma_channel);
    assert(ret == ESP_OK);

    return ESP_OK != ret;
}

