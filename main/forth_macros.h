#define NEXT do { W = *IP++; goto *W; } while (0)
#define PUSHD *(--DSP)=T
#define POPD T=*(DSP++)
