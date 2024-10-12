#include "forth_macros.h"
#include "registers.h"
#include "docol.h"

void macros(int i) {
   switch (i) {
      case 1:
         asm(".macro PUSHD");
         PUSHD;
         asm(".endm");
         NEXT;
         break;

      case 2:
        do {
         __label__ not_eliminate;\
         asm goto (""::::not_eliminate);\
         asm(".macro NEXT");
         NEXT;
         not_eliminate:\
         asm(".endm");
         break;
         } while (0);

      case 3:
         asm(".macro POPD");
         POPD;
         asm(".endm");
         NEXT;
         break;
   }
}
