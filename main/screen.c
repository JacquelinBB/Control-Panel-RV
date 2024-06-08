#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "screen.h"
#include "mqtt_cloud.h"
#include "sensor_hcsr04.h"
#include "sensor_bme280.h"
#include "water_pump.h"
#include "sensor_mq2.h"

/* Littlevgl specific */
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "lvgl_helpers.h"

bool is_pump_on = false;

static void button_event_handler_home(lv_obj_t * btn, lv_event_t event);
static void button_event_handler_door(lv_obj_t * btn, lv_event_t event);
static void button_event_handler_pantry_1(lv_obj_t * btn, lv_event_t event);
static void button_event_handler_pantry_2(lv_obj_t * btn, lv_event_t event);
static void button_event_handler_attic_1(lv_obj_t * btn, lv_event_t event);
static void button_event_handler_attic_2(lv_obj_t * btn, lv_event_t event);
void update_info_text1(lv_task_t * task);
void update_info_text2(lv_task_t * task);
void update_info_text3(lv_task_t * task);
void update_info_text4(lv_task_t * task);
static void home_create(lv_obj_t * parent);
static void door_create(lv_obj_t * parent);
static void pantry_create(lv_obj_t * parent);
static void attic_create(lv_obj_t * parent);
#if LV_USE_THEME_MATERIAL
static void color_chg_event_cb(lv_obj_t * sw, lv_event_t e);
#endif

static lv_obj_t * tv;
static lv_obj_t * t1;
static lv_obj_t * t2;
static lv_obj_t * t3;
static lv_obj_t * t4;

static lv_obj_t *label1;
static lv_obj_t *label2;
static lv_obj_t *label3;
static lv_obj_t *label4;

static lv_style_t style_box;

SemaphoreHandle_t xGuiSemaphore;

void guiTask(void *pvParameter) {

    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    lvgl_driver_init();

    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);

    static lv_color_t *buf2 = NULL;

    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    create_demo_application();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));

        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }

    free(buf1);
    vTaskDelete(NULL);
}

void create_demo_application(void)
{
    tv = lv_tabview_create(lv_scr_act(), NULL);

    if(LV_THEME_DEFAULT_INIT == lv_theme_material_init) {
        lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
        if(disp_size >= LV_DISP_SIZE_MEDIUM) {
            lv_obj_set_style_local_pad_left(tv, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, LV_HOR_RES / 3);
            lv_obj_t * sw = lv_switch_create(lv_scr_act(), NULL);
            if(lv_theme_get_flags() & LV_THEME_MATERIAL_FLAG_DARK)
                lv_switch_on(sw, LV_ANIM_OFF);
            lv_obj_set_event_cb(sw, color_chg_event_cb);
            lv_obj_set_pos(sw, LV_DPX(10), LV_DPX(17));
            lv_obj_set_style_local_value_str(sw, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, "Dark");
            lv_obj_set_style_local_value_align(sw, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_RIGHT_MID);
            lv_obj_set_style_local_value_ofs_x(sw, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, LV_DPI/35);
        }
    }

    t1 = lv_tabview_add_tab(tv, "Casa");
    t2 = lv_tabview_add_tab(tv, "Porta");
    t3 = lv_tabview_add_tab(tv, "Despensa");
    t4 = lv_tabview_add_tab(tv, "Sotao");

    lv_style_init(&style_box);
    lv_style_set_value_align(&style_box, LV_STATE_DEFAULT, LV_ALIGN_OUT_TOP_LEFT);
    lv_style_set_value_ofs_y(&style_box, LV_STATE_DEFAULT, - LV_DPX(10));
    lv_style_set_margin_top(&style_box, LV_STATE_DEFAULT, LV_DPX(35));

    home_create(t1);
    door_create(t2);
    pantry_create(t3);
    attic_create(t4);
}

void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}


static void home_create(lv_obj_t * parent)
{
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);
    lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
    lv_coord_t grid_w = lv_page_get_width_grid(parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1);

    lv_obj_t * h_ambiente = lv_cont_create(parent, NULL);
    lv_obj_set_style_local_value_str(h_ambiente, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Ambiente");
    lv_cont_set_fit2(h_ambiente, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_add_style(h_ambiente, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_width(h_ambiente, grid_w);

    label1 = lv_label_create(h_ambiente, NULL);
    lv_label_set_text(label1, "Esperando algum dado para ser exibido...");

    lv_task_create(update_info_text1, 2000, LV_TASK_PRIO_LOW, NULL);
    
    lv_obj_t * h_bomba = lv_cont_create(parent, NULL);
    lv_cont_set_layout(h_bomba, LV_LAYOUT_PRETTY_MID);
    lv_obj_add_style(h_bomba, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_style_local_value_str(h_bomba, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Bomba de Agua");
    lv_cont_set_fit2(h_bomba, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_set_width(h_bomba, grid_w); 

    lv_obj_t * btn = lv_btn_create(h_bomba, NULL);
    lv_btn_set_checkable(btn, true);
    lv_btn_set_fit2(btn, LV_FIT_NONE, LV_FIT_NONE);
    lv_obj_set_size(btn, 115, 50);
    lv_obj_set_event_cb(btn, button_event_handler_home);
    
    lv_obj_t * label = lv_label_create(btn, NULL);
    lv_label_set_text(label, "Desligado");

    lv_obj_t * h_agua = lv_cont_create(parent, NULL);
    lv_obj_set_style_local_value_str(h_agua, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Tanque de agua");
    lv_cont_set_fit2(h_agua, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_add_style(h_agua, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_width(h_agua, grid_w);

    label3 = lv_label_create(h_agua, NULL);
    lv_label_set_text(label3, "Esperando algum dado para ser exibido...");

    lv_task_create(update_info_text3, 2000, LV_TASK_PRIO_LOW, NULL);
}

static void door_create(lv_obj_t * parent)
{
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);
    lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
    lv_coord_t grid_w = lv_page_get_width_grid(parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1);

    lv_obj_t * h_porta_data = lv_cont_create(parent, NULL);
    lv_obj_set_style_local_value_str(h_porta_data, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Porta de Entrada");
    lv_cont_set_fit2(h_porta_data, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_add_style(h_porta_data, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_width(h_porta_data, grid_w);

    label2 = lv_label_create(h_porta_data, NULL);
    lv_label_set_text(label2, "Esperando algum dado para ser exibido...");

    lv_task_create(update_info_text2, 2000, LV_TASK_PRIO_LOW, NULL);

    lv_obj_t * h_porta_action = lv_cont_create(parent, NULL);
    lv_obj_set_style_local_value_str(h_porta_action, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Porta");
    lv_cont_set_fit2(h_porta_action, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_add_style(h_porta_action, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_width(h_porta_action, grid_w);

    lv_obj_t * btn = lv_btn_create(h_porta_action, NULL);
    lv_btn_set_checkable(btn, true);
    lv_btn_set_fit2(btn, LV_FIT_NONE, LV_FIT_NONE);
    lv_obj_set_size(btn, 100, 50);
    lv_obj_set_event_cb(btn, button_event_handler_door);
    
    lv_obj_t * label = lv_label_create(btn, NULL);
    lv_label_set_text(label, "Abrir");
}

static void pantry_create(lv_obj_t * parent)
{
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);
    lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
    lv_coord_t grid_w = lv_page_get_width_grid(parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1);

    lv_obj_t * h_ambiente = lv_cont_create(parent, NULL);
    lv_obj_set_style_local_value_str(h_ambiente, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Balinha");
    lv_cont_set_fit2(h_ambiente, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_add_style(h_ambiente, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_width(h_ambiente, grid_w);

    lv_obj_t * btn = lv_btn_create(h_ambiente, NULL);
    lv_btn_set_checkable(btn, true);
    lv_btn_set_fit2(btn, LV_FIT_NONE, LV_FIT_NONE);
    lv_obj_set_size(btn, 150, 50);
    lv_obj_set_event_cb(btn, button_event_handler_pantry_1);
    
    lv_obj_t * label = lv_label_create(btn, NULL);
    lv_label_set_text(label, "Liberar");

    lv_obj_t * h_drink = lv_cont_create(parent, NULL);
    lv_obj_set_style_local_value_str(h_drink, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Fanta");
    lv_cont_set_fit2(h_drink, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_add_style(h_drink, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_width(h_drink, grid_w);

    lv_obj_t * btn2 = lv_btn_create(h_drink, NULL);
    lv_btn_set_checkable(btn2, true);
    lv_btn_set_fit2(btn2, LV_FIT_NONE, LV_FIT_NONE);
    lv_obj_set_size(btn2, 150, 50);
    lv_obj_set_event_cb(btn2, button_event_handler_pantry_2);
    
    lv_obj_t * label2 = lv_label_create(btn2, NULL);
    lv_label_set_text(label2, "Liberar");
}

static void attic_create(lv_obj_t * parent){
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);
    lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
    lv_coord_t grid_w = lv_page_get_width_grid(parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1);

    lv_obj_t * h_ambiente = lv_cont_create(parent, NULL);
    lv_obj_set_style_local_value_str(h_ambiente, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Analise da Agua");
    lv_cont_set_fit2(h_ambiente, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_add_style(h_ambiente, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_width(h_ambiente, grid_w);

    label4 = lv_label_create(h_ambiente, NULL);
    lv_label_set_text(label4, "Esperando algum dado para ser exibido...");

    lv_task_create(update_info_text4, 2000, LV_TASK_PRIO_LOW, NULL);

    lv_obj_t * h_bomba = lv_cont_create(parent, NULL);
    lv_obj_set_style_local_value_str(h_bomba, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Bomba");
    lv_cont_set_fit2(h_bomba, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_add_style(h_bomba, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_width(h_bomba, grid_w);

    lv_obj_t * btn1 = lv_btn_create(h_bomba, NULL);
    lv_btn_set_checkable(btn1, true);
    lv_btn_set_fit2(btn1, LV_FIT_NONE, LV_FIT_NONE);
    lv_obj_set_size(btn1, 165, 50);
    lv_obj_set_event_cb(btn1, button_event_handler_attic_1);
    
    lv_obj_t * label1 = lv_label_create(btn1, NULL);
    lv_label_set_text(label1, "Desligado");

    lv_obj_t * h_valvula = lv_cont_create(parent, NULL);
    lv_obj_set_style_local_value_str(h_valvula, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Valvula");
    lv_cont_set_fit2(h_valvula, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_add_style(h_valvula, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_width(h_valvula, grid_w);

    lv_obj_t * btn2 = lv_btn_create(h_valvula, NULL);
    lv_btn_set_checkable(btn2, true);
    lv_btn_set_fit2(btn2, LV_FIT_NONE, LV_FIT_NONE);
    lv_obj_set_size(btn2, 165, 50);
    lv_obj_set_event_cb(btn2, button_event_handler_attic_2);
    
    lv_obj_t * label2 = lv_label_create(btn2, NULL);
    lv_label_set_text(label2, "Desligado");
}

void update_info_text1(lv_task_t * task) {
    char info_text1[256];
    snprintf(info_text1, sizeof(info_text1), 
         "Temperatura: %.2f °C\n"
         "Umidade: %.2f %%\n"
         "Pressao Barometrica: %.2f hPa\n"
         "Presenca de Gas: %.1f ppm", 
         temp, hum, press, mq2_value);
    lv_label_set_text(label1, info_text1);
}

void update_info_text3(lv_task_t * task) {
    char info_text3[30];
    snprintf(info_text3, sizeof(info_text3), 
         "Nivel de agua: %.2f", 
         water_level);
    lv_label_set_text(label3, info_text3);
}

void update_info_text2(lv_task_t * task) {
    if(topico_door == true || topico_open_door == true)
    {
        char info_door[10];
        if(get_porta == 0)
        {
            snprintf(info_door, sizeof(info_door), "Fechado");
        }
        else if(get_porta == 1)
        {
            snprintf(info_door, sizeof(info_door), "Aberto");
        }

        char time_str[15];
        int total_seconds = get_acesso_time / 1000;
        int hours = total_seconds / 3600;
        int minutes = (total_seconds % 3600) / 60;
        int seconds = total_seconds % 60;

        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", hours, minutes, seconds);

        char info_text21[128];
        snprintf(info_text21, sizeof(info_text21),  
             "Estado da porta: %s\n"
             "Ultimo acesso registrado: %s\n"
             "Ultimo acesso por: %s",
             info_door, time_str, get_name);
        
        lv_label_set_text(label2, info_text21);
        topico_door = false;
        topico_open_door = false;
    }
}

void update_info_text4(lv_task_t * task) {
    if(topico_water == true)
    {
        char info_text4[128];
        snprintf(info_text4, sizeof(info_text4),  
             "Quantidade de agua na cisterna: %.2f\n"
             "Quantidade de agua na caixa: %.2f\n"
             "Vazao da agua: %d",
             get_cisterna, get_caixa, get_agua);
        lv_label_set_text(label4, info_text4);

        topico_water = false;
    }
}

static void button_event_handler_home(lv_obj_t * btn, lv_event_t event)
{   
    if (event == LV_EVENT_PRESSED) {
        const char * current_text = lv_label_get_text(lv_obj_get_child(btn, 0));

        if ((strcmp(current_text, "Desligado") == 0)) {
            lv_label_set_text(lv_obj_get_child(btn, 0), "Ligado");  
            gpio_set_level(RELAY_PIN, 1);
        } else {
            lv_label_set_text(lv_obj_get_child(btn, 0), "Desligado");   
            gpio_set_level(RELAY_PIN, 0);
        }
        is_pump_on = !is_pump_on;
        char message[60];
        snprintf(message, sizeof(message), "Water pump is %s\n", is_pump_on ? "on" : "off");
        mqtt_publish_message("rv/pump", message);
    }
}

static void button_event_handler_door(lv_obj_t * btn, lv_event_t event)
{
    if (event == LV_EVENT_PRESSED) {
        mqtt_publish_message("esp32/open_door", "111111111111111111111111111"); 
    }
}

static void button_event_handler_pantry_1(lv_obj_t * btn, lv_event_t event)
{
    if (event == LV_EVENT_PRESSED) {
        char json_message[12];
        int type = 1;
        snprintf(json_message, sizeof(json_message), "{\"type\": %d}", type);
        mqtt_publish_message("set_food", json_message);
        
    }
}

static void button_event_handler_pantry_2(lv_obj_t * btn, lv_event_t event)
{
    if (event == LV_EVENT_PRESSED) {
        mqtt_publish_message("set_drink", "1");
    }
}

static void button_event_handler_attic_1(lv_obj_t * btn, lv_event_t event)
{
    if (event == LV_EVENT_PRESSED) {
        const char * current_text = lv_label_get_text(lv_obj_get_child(btn, 0));

        if ((strcmp(current_text, "Desligado") == 0)) {
            lv_label_set_text(lv_obj_get_child(btn, 0), "Ligado"); 
            mqtt_publish_message("sala11/bomba", "1"); 
        } else {
            lv_label_set_text(lv_obj_get_child(btn, 0), "Desligado");
            mqtt_publish_message("sala11/bomba", "0");   
        }
    }
}

static void button_event_handler_attic_2(lv_obj_t * btn, lv_event_t event)
{
    if (event == LV_EVENT_PRESSED) {
        const char * current_text = lv_label_get_text(lv_obj_get_child(btn, 0));

        if ((strcmp(current_text, "Desligado") == 0)) {
            lv_label_set_text(lv_obj_get_child(btn, 0), "Ligado"); 
            mqtt_publish_message("sala11/valvula", "1");
        } else {
            lv_label_set_text(lv_obj_get_child(btn, 0), "Desligado");
            mqtt_publish_message("sala11/valvula", "0");   
        }
    }
}

static void color_chg_event_cb(lv_obj_t * sw, lv_event_t e)
{
    if(LV_THEME_DEFAULT_INIT != lv_theme_material_init) return;
    if(e == LV_EVENT_VALUE_CHANGED) {
        uint32_t flag = LV_THEME_MATERIAL_FLAG_LIGHT;
        if(lv_switch_get_state(sw)) flag = LV_THEME_MATERIAL_FLAG_DARK;

        LV_THEME_DEFAULT_INIT(lv_theme_get_color_primary(), lv_theme_get_color_secondary(),
                flag,
                lv_theme_get_font_small(), lv_theme_get_font_normal(), lv_theme_get_font_subtitle(), lv_theme_get_font_title());
    }
}