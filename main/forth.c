#include "forth.h"
#include "forth_prims.h"
#include "esp_timer.h"

//stacks
int dstack[dstack_size];
int rstack[rstack_size];
int dict[dict_size];

void app_main() {
   extern const int START;
   const int _start[]={&START};
   DSP=&dstack[dstack_size];
   RSP=&rstack[rstack_size];
   here = &dict;
   IP=_start;
   NEXT;
}


int get_ms() {
   return esp_timer_get_time();
}
