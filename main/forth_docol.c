#include "forth_prims.h"
#include "registers.h"
#include "docol.h"

void docol(int i) {
#include "forth_macros.s"
   switch (i) {
      case 0:
         asm ("docol_start:");\
         _DOCOL;
         asm("docol_end:");\
         asm(".global docol_len \n .equ docol_len, . - docol_start");\
         break;
   }
}
