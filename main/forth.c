#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "forth.h"
#include "forth_prims.h"
#include "registers.h"
#include "esp_timer.h"
#include "uart.h"

#define READ_TASK_STACK_SIZE 4096
int *here;
int *latest;

//stacks
int dstack[dstack_size];
int *rstack[rstack_size];
int dict[dict_size];

void app_main() {
   extern const int INTERPRET;
   extern const int link_switch_context;
   const int *_start[]={&INTERPRET};

	ESP_ERROR_CHECK(uart_start());
   xTaskCreate(&uart_read_task, "uart_read_task", READ_TASK_STACK_SIZE, NULL, 10, NULL);
   DSP=&dstack[dstack_size];
   RSP=&rstack[rstack_size];
   here = (int*)&dict;
   latest = (int*)&link_switch_context;
   IP=(int **)_start;
   NEXT;
}


int get_ms() {
   return esp_timer_get_time();
}
