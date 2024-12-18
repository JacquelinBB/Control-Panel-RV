/*
 * Este arquivo está licenciado sob a Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0).
 * Você pode compartilhar, modificar e usar este trabalho para fins não comerciais, desde que atribua os devidos créditos aos autores.
 * Para mais informações sobre a licença, visite: https://creativecommons.org/licenses/by-nc/4.0/
 */

#ifndef SCREEN
#define SCREEN

#define TAG "demo"
#define LV_TICK_PERIOD_MS 1

extern bool is_pump_on;

void lv_tick_task(void *arg);
void guiTask(void *pvParameter);
void create_demo_application(void);

#endif
