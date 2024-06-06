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

/* Littlevgl specific */
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "lvgl_helpers.h"

static int var1 = 10;
static int var2 = 20;
static int var3 = 30;
static int var4 = 40;

static void btn_event_handler(lv_obj_t * btn, lv_event_t event);
static void button_event_handler_home(lv_obj_t * btn, lv_event_t event);
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

    char info_text1[256];
    snprintf(info_text1, sizeof(info_text1), 
             "Temperatura: %d °C\n"
             "Umidade: %d %%\n"
             "Pressao Barometrica: %d hPa\n"
             "Presenca de Gas: %d ppm", 
             var1, var2, var3, var4);

    lv_obj_t * label1 = lv_label_create(h_ambiente, NULL);
    lv_label_set_text(label1, info_text1);
    
    lv_obj_t * h_bomba = lv_cont_create(parent, NULL);
    lv_cont_set_layout(h_bomba, LV_LAYOUT_PRETTY_MID);
    lv_obj_add_style(h_bomba, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_style_local_value_str(h_bomba, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Bomba de Agua");
    lv_cont_set_fit2(h_bomba, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_set_width(h_bomba, grid_w); 

    lv_obj_t * btn = lv_btn_create(h_bomba, NULL);
    lv_btn_set_checkable(btn, true);
    lv_btn_set_fit2(btn, LV_FIT_NONE, LV_FIT_NONE);
    lv_obj_set_size(btn, 125, 50);
    lv_obj_set_event_cb(btn, button_event_handler_home);
    
    lv_obj_t * label = lv_label_create(btn, NULL);
    lv_label_set_text(label, "Ativar");
}

static void door_create(lv_obj_t * parent)
{
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);
    lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
    lv_coord_t grid_w = lv_page_get_width_grid(parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1);

    lv_obj_t * h_ambiente = lv_cont_create(parent, NULL);
    lv_obj_set_style_local_value_str(h_ambiente, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Ambiente");
    lv_cont_set_fit2(h_ambiente, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_add_style(h_ambiente, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_width(h_ambiente, grid_w);

    if(topico_door == true)
    {
        topico_door = false;
    }
    if(topico_open_door == true)
    {
        topico_open_door = false;
    }

    char info_text1[256];
    snprintf(info_text1, sizeof(info_text1), 
             "Status da porta: %d\n"
             "Último acesso registrado: %d\n"
             "Último acesso por: %s",
             get_porta, get_acesso_time, get_name);

    lv_obj_t * label1 = lv_label_create(h_ambiente, NULL);
    lv_label_set_text(label1, info_text1);
}

static void pantry_create(lv_obj_t * parent)
{
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_MID);

    lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
    lv_coord_t grid_h = lv_page_get_height_grid(parent, 1, 1);
    lv_coord_t grid_w;
    if(disp_size <= LV_DISP_SIZE_SMALL) grid_w = lv_page_get_width_grid(parent, 1, 1);
    else grid_w = lv_page_get_width_grid(parent, 2, 1);

    // Create the first page with text
    lv_obj_t * page1 = lv_page_create(parent, NULL);
    lv_obj_set_size(page1, grid_w, grid_h);
    lv_page_set_scroll_propagation(page1, true);

    if(topico_food == true)
    { // x y z
        topico_food = false;
    }
    if(topico_drink == true)
    {
        topico_drink = false;
    }

    // Creating the text with the values of the variables
    char info_text[256];
    snprintf(info_text, sizeof(info_text), 
             "Status da quantidade de comida no tipo 1: %s\n"
             "Status da quantidade de comida no tipo 2: %s\n"
             "Status da quantidade de comida no tipo 3: %s\n"
             "Status da quantidade de líquido disponivel: %s\n",
             get_food1, get_food2, get_food3, get_drink);

    lv_obj_t * label1 = lv_label_create(page1, NULL);
    lv_label_set_text(label1, info_text);
    //lv_obj_set_style_local_text_font(label1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_22); 

    // Create a second page for buttons
    lv_obj_t * page2 = lv_page_create(parent, NULL);
    lv_obj_set_size(page2, grid_w, grid_h);
    lv_page_set_scroll_propagation(page2, true);

    // Names for the buttons
    const char* btn_names[4] = {"Agua", "Macarrao", "Arroz", "Feijao"};

    // Create buttons inside the second page
    for (int i = 0; i < 4; i++) {
        lv_obj_t * btn = lv_btn_create(page2, NULL);
        lv_obj_set_size(btn, grid_w / 2, grid_h / 4); // Adjust the size of the buttons as needed

        lv_obj_t * label = lv_label_create(btn, NULL);
        lv_label_set_text(label, btn_names[i]);

        lv_obj_set_event_cb(btn, btn_event_handler); // Set a generic event handler for buttons
    }
}

static void attic_create(lv_obj_t * parent){
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY_TOP);
    lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
    lv_coord_t grid_w = lv_page_get_width_grid(parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1);

    lv_obj_t * h_ambiente = lv_cont_create(parent, NULL);
    lv_obj_set_style_local_value_str(h_ambiente, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, "Ambiente");
    lv_cont_set_fit2(h_ambiente, LV_FIT_TIGHT, LV_FIT_TIGHT);
    lv_obj_add_style(h_ambiente, LV_CONT_PART_MAIN, &style_box);
    lv_obj_set_width(h_ambiente, grid_w);

    if(topico_water == true)
    {
        topico_water = false;
    }
    if(topico_fluxo == true)
    {
        topico_fluxo = false;
    }

    char info_text1[256];
    snprintf(info_text1, sizeof(info_text1), 
             "Status do time: %d\n"
             "Status do led: %d\n"
             "Status da Distância: %.2f cm\n"
             "Status do Nível de água: %d\n"
             "Status do Volume da cisterna: %.2f\n"
             "Status do Volume da caixa: %.2f\n"
             "Status do Tempo do Fluxo: %d\n"
             "Status do Fluxo de água: %d\n",
             get_time, get_led, get_distance, get_agua, get_cisterna, get_caixa, get_fluxo_time, get_fluxo);

    lv_obj_t * label1 = lv_label_create(h_ambiente, NULL);
    lv_label_set_text(label1, info_text1);
}

static void btn_event_handler(lv_obj_t * btn, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        // Add actions for buttons here
        printf("Button clicked\n");
    }
}

static void button_event_handler_home(lv_obj_t * btn, lv_event_t event)
{   
    if (event == LV_EVENT_CLICKED) {
        const char * current_text = lv_label_get_text(lv_obj_get_child(btn, 0));

        if (strcmp(current_text, "Ativar") == 0) {
            lv_label_set_text(lv_obj_get_child(btn, 0), "Desativar");  
            printf("Botão clicado e está ativado\n");
        } else {
            lv_label_set_text(lv_obj_get_child(btn, 0), "Ativar");   
            printf("Botão clicado e está desativado\n");
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