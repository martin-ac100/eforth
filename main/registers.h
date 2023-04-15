//registers assignement
register int **IP asm("a2"); // instruction pointer
register int *W asm("a7"); // actual word
register int T asm("a3"); // top of data stack = S0
register int *DSP asm("a4"); // data stack pointer
register int **RSP asm("a5"); // return stack pointer
register int X asm("a6"); // scratch register

