#include "forth_prims.h"

void docol(int i) {
   switch (i) {
      case 0:
         asm(".align 4");
         asm ("docol_start:");\
         _DOCOL;
         asm("docol_end:");\
         asm(".global docol_len \n .equ docol_len, . - docol_start");\
         break;
   }
}
