#ifndef SCREEN
#define SCREEN

#define TAG "demo"
#define LV_TICK_PERIOD_MS 1

void lv_tick_task(void *arg);
void guiTask(void *pvParameter);
void create_demo_application(void);

#endif