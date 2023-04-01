
//registers assignement
register int **IP asm("a2"); // instruction pointer
register int *W asm("a7"); // actual word
register int T asm("a3"); // top of data stack = S0
register int *DSP asm("a4"); // data stack pointer
register int **RSP asm("a5"); // return stack pointer
register int X asm("a6"); // scratch register

typedef struct task_t {
   int wake_at_ms;
   int *semaphore;
   int priority;
   struct task_t *next_task;
   int *dsp;
   int **rsp;
} task_t;

typedef struct {
   int c_pos;
   int c_top;
   int w_pos;
   char *w_buff;
   char *c_buff;
} io_buff_t;


#include "docol.h"
#define _DOCOL do {\
         __label__ not_eliminate;\
         asm goto (""::::not_eliminate);\
         *(--RSP) = (int *)IP;\
         IP = (int **)( (int)W + docol_len);\
         NEXT;\
      not_eliminate:\
         asm(".align 4");\
         } while (0)

#define NEXT do { W = *IP++; goto *W; } while (0)
#define PUSHD *(--DSP)=T
#define POPD T=*(DSP++)
#define S1 *DSP
#define S2 *(DSP+1)
