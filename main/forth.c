#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "forth.h"
#include "forth_prims.h"
#include "registers.h"
#include "esp_timer.h"

#define READ_TASK_STACK_SIZE 1024
extern void uart_read_task;
extern int *here;

//stacks
int dstack[dstack_size];
int rstack[rstack_size];
int dict[dict_size];

void app_main() {
   extern const int INTERPRET;
   const int _start[]={&INTERPRET};
   xTaskCreate(&uart_read_task, "uart_read_task", READ_TASK_STACK_SIZE, NULL, 10, NULL);
   DSP=&dstack[dstack_size];
   RSP=&rstack[rstack_size];
   here = &dict;
   IP=_start;
   NEXT;
}


int get_ms() {
   return esp_timer_get_time();
}
