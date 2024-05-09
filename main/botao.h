#ifndef BUTTON
#define BUTTON

void uart_init();
char read_key();
void register_key_callback(void (*callback)(char key));
void key_task(void *params);

#endif
