typedef struct task_t {
   int wake_at_ms;
   int *semaphore;
   int priority;
   struct task_t *next_task;
   int *dsp;
   int **rsp;
} task_t;

typedef struct forth_input_buffer_t {
        char chars[256];
        int unread;
        char *read_pos;
} forth_input_buffer_t;

#include "docol.h"
#define _DOCOL do {\
         __label__ not_eliminate;\
         asm goto (""::::not_eliminate);\
         *(--RSP) = (int *)IP;\
         IP = (int **)( (int)W + docol_len);\
         NEXT;\
      not_eliminate:\
         } while (0)

extern int dstack[];
#define NEXT asm("NEXT")
#define PUSHD asm("PUSHD")
#define POPD asm("POPD")
#define S1 *DSP
#define S2 *(DSP+1)
