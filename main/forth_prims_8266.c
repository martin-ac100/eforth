#include "forth.h"

//registers assignement
register int *IP asm("a2"); // instruction pointer
register int W asm("a0"); // actual word
register int T asm("a3"); // top of data stack = S0
register int *DSP asm("a4"); // data stack pointer
register int **RSP asm("a5"); // return stack pointer
register int X asm("a6"); // scratch register

int *here;
int compiling;
int *latest;

char uart_wordbuffer[256];
//char enow_wordbuffer[256];
char wordbuffer[32];

typedef struct {
	int pos;
	char *buf;
} t_read_buf;

t_read_buf uart_buf = {.pos = 0, .buf = uart_wordbuffer};


#define WORD(x) asm(x":")
#define NEXT W=*(IP++); asm("jx a0")
#define PUSHD *(--DSP)=T
#define POPD T=*(DSP++)
#define S1 *DSP
#define S2 *(DSP+1)

void EXIT() {
	IP = *RSP++;
	NEXT;
}

void DUP() {
	WORD("NEW_DUP");
	PUSHD;
	NEXT;
}

void DDUP() {
	*(DSP-1) = T;
	*(DSP-2) = S1;
	DSP = DSP -2;
	NEXT;
}


void DROP() {
	POPD;
	NEXT;
}

void OVER() {
	PUSHD;
	T = S2;
	NEXT;
}

void SWAP() {
	X = S1;
	S1 = T;
	T = X;
	NEXT;
}

void ROT() {
	X = T;
	T = S2;
	S2 = S1;
	S1 = X;
	NEXT;
}

void ADD() {
	T = T + *(DSP++);
	NEXT;
}

void SUB() {
	T = *(DSP++) - T;
	NEXT;
}

void MUL() {
	T = *(DSP++) * T;
	NEXT;
}

void DIV() {
	T = *(DSP++) / T;
	NEXT;
}

void MOD() {
	T = *(DSP++) % T;
	NEXT;
}

void EQU() {
	T = *(DSP++) == T ? 1 : 0;
	NEXT;
}

void LT() {
	T = *(DSP++) < T ? 1 : 0;
	NEXT;
}

void LTE() {
	T = *(DSP++) <= T ? 1 : 0;
	NEXT;
}

void GT() {
	T = *(DSP++) > T ? 1 : 0;
	NEXT;
}

void GTE() {
	T = *(DSP++) >= T ? 1 : 0;
	NEXT;
}

void EQZ() {
	T = T == 0 ? 1 : 0;
	NEXT;
}

void GTZ() {
	T = T > 0 ? 1 : 0;
	NEXT;
}

void GEZ() {
	T = T >= 0 ? 1 : 0;
	NEXT;
}

void LTZ() {
	T = T < 0 ? 1 : 0;
	NEXT;
}

void LEZ() {
	T = T <= 0 ? 1 : 0;
	NEXT;
}

void LIT() {
	PUSHD;
	T = *(IP++);
	NEXT;
}

void JZ() {
	if (T) {
		IP++;
	}
	else {
		IP = (int *) *IP;
	}
	NEXT;
}

void TOR() {
	*(--RSP) = (int *)T;
	POPD;
	NEXT;
}

void FROMR() {
	PUSHD;
	T = *(int *)(RSP++);
	NEXT;
}

void FETCH() {
	T = *(int *)T;
	NEXT;
}

void STORE() {
	*(int*)T = *DSP++;
	POPD;
	NEXT;
}

void FETCHBYTE() {
	T = *(char *)T;
	NEXT;
}

void STOREBYTE() {
	*(char *)T = (char)(*DSP++);
	POPD;
	NEXT;
}

void DUPPLUS() { // { a -- a+1 a
	PUSHD;
	S1 = S1 + 1;
	NEXT;
}

void OVERPLUS() { // a b -- a+1 b a
	PUSHD;
	T = S2;
	S2 = S2 + 1;
	NEXT;
}

void COMMA() {
	*(here++) = T;
	POPD;
	NEXT;
}
