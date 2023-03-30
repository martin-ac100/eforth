#include "forth.h"
#include "forth_prims.h"
#include "esp_timer.h"

//stacks
int dstack[dstack_size];
int rstack[rstack_size];

void app_main() {
   extern const int TST;
   const int _start[]={&TST};
   DSP=&dstack[dstack_size];
   RSP=&rstack[rstack_size];
   IP=_start;
   NEXT;
}


int get_ms() {
   return esp_timer_get_time();
}
