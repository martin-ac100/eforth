#include "forth.h"

//stacks
int dstack[dstack_size];
int rstack[rstack_size];

//registers assignement
register int *IP asm("a2"); // instruction pointer
register int W asm("a0"); // actual word
register int T asm("a3"); // top of data stack
register int *DSP asm("a4"); // data stack pointer
register int *RSP asm("a5"); // return stack pointer
register int X asm("a6"); // scratch register

#define NEXT W=*IP++; asm("jx a0")
#define PUSHD *(++DSP)=T;
#define POPD T=*(DSP--);

