#include "forth.h"
extern int get_ms();

typedef struct task_t {
	int wake_at_ms;
	int *semaphore;
	int priority;
	struct task_t *next_task;
	int *dsp;
	int ***rsp;
} task_t;

typedef struct {
	int c_pos;
	int c_top;
	int w_pos;
	char *w_buff;
	char *c_buff;
} io_buff_t;

#define _iskey rb->c_pos < rb->c_top
#define _readkey rb->c_buff[rb->c_pos++]
#define _key PUSHD;T=0;if (_iskey) {T=_readkey;PUSHD;T=1;} else PUSHD; if (!_iskey) {rb->c_pos = 0; rb->c_top = 0;}
		

//registers assignement
register int **IP asm("a2"); // instruction pointer
register int *W asm("a0"); // actual word
register int T asm("a3"); // top of data stack = S0
register int *DSP asm("a4"); // data stack pointer
register int ***RSP asm("a5"); // return stack pointer
register int X asm("a6"); // scratch register

int here;
int compiling;
int latest;
int base;
static char[] digits="0123456789ABCDEF";

task_t *first_task;
task_t *task;

io_buff_t *rb;

char uart_cb[256];
char uart_wb[32];
//char enow_rb[256];
//char enow_wb[32];

io_buff_t uart_rb = {.c_pos = 0, .c_top = sizeof(uart_rb), .w_pos = 0, .w_buff = uart_wb, .c_buff = uart_cb};



#define NEXT W=*(IP++); goto **W
#define PUSHD *(--DSP)=T
#define POPD T=*(DSP++)
#define S1 *DSP
#define S2 *(DSP+1)

#define def_word(name,label,flags) case __COUNTER__: \
	asm(".global "label"\n"\
	"link_"label":\n"\
	".int link,2f-1f+"flags"\n"\
	".set link,link_"label"\n"\
	"1:\n"\
	".asciz \""name"\"\n"\
	"2:\n"\
	".align 4\n"\
	label":");

#define def_code_word(name,label,flags) def_word(label,name,flags); asm(".int 3f\n3:");
#define exe(word)	do {__label__ newIP;\
								IP = &&newIP;\
								NEXT;
								newIP:\
								asm(".int "words",1f\n1:")

void prims(int c) {
		asm(".set link,0");
	switch (c) {

	def_code_word("EXIT","EXIT","0")
		IP = *RSP++;
		NEXT;

	def_code_word("DUP","DUP","0")
		PUSHD;
		NEXT;

	def_code_word("DUP+","DUP+","0")
		X = T;
		T = T + 1;
		PUSHD;
		T = X;
		NEXT;

	def_code_word("DUP-","DUP-","0")
		X = T;
		T = T - 1;
		PUSHD;
		T = X;
		NEXT;

	def_code_word("DDUP","DDUP","0")
		*(DSP-1) = T;
		*(DSP-2) = S1;
		DSP = DSP -2;
		NEXT;


	def_code_word("DROP","DROP","0")
		POPD;
		NEXT;

	def_code_word("OVER","OVER","0")
		PUSHD;
		T = S2;
		NEXT;

	def_code_word("SWAP","SWAP","0")
		X = S1;
		S1 = T;
		T = X;
		NEXT;

	def_code_word("ROT","ROT","0")
		X = T;
		T = S2;
		S2 = S1;
		S1 = X;
		NEXT;

	def_code_word("+","ADD","0")
		T = T + *(DSP++);
		NEXT;

	def_code_word("-","SUB","0")
		T = *(DSP++) - T;
		NEXT;

	def_code_word("*","MUL","0")
		T = *(DSP++) * T;
		NEXT;

	def_code_word("/","DIV","0")
		T = *(DSP++) / T;
		NEXT;

	def_code_word("MOD","MOD","0")
		T = *(DSP++) % T;
		NEXT;

	def_code_word("=","EQU","0")
		T = *(DSP++) == T ? 1 : 0;
		NEXT;

	def_code_word("<","LT","0")
		T = *(DSP++) < T ? 1 : 0;
		NEXT;

	def_code_word("<=","LTE","0")
		T = *(DSP++) <= T ? 1 : 0;
		NEXT;

	def_code_word(">","GT","0")
		T = *(DSP++) > T ? 1 : 0;
		NEXT;

	def_code_word(">=","GTE","0")
		T = *(DSP++) >= T ? 1 : 0;
		NEXT;

	def_code_word("=0","EQZ","0")
		T = T == 0 ? 1 : 0;
		NEXT;

	def_code_word(">0","GTZ","0")
		T = T > 0 ? 1 : 0;
		NEXT;

	def_code_word(">=0","GEZ","0")
		T = T >= 0 ? 1 : 0;
		NEXT;

	def_code_word("<0","LTZ","0")
		T = T < 0 ? 1 : 0;
		NEXT;

	def_code_word("<=0","LEZ","0")
		T = T <= 0 ? 1 : 0;
		NEXT;

	def_code_word("LIT","LIT","0")
		PUSHD;
		T = (int)*(IP++);
		NEXT;

	def_code_word("JZ","JZ","0")
		if (T) {
			IP++;
		}
		else {
			IP = (int **) *IP;
		}
		NEXT;

	def_code_word(">R","TOR","0")
		*(--RSP) = (int **)T;
		POPD;
		NEXT;

	def_code_word("R>","FROMR","0")
		PUSHD;
		T = *(int *)(RSP++);
		NEXT;

	def_code_word("@","FETCH","0")
		T = *(int *)T;
		NEXT;

	def_code_word("!","STORE","0")
		*(int*)T = *DSP++;
		POPD;
		NEXT;

	def_code_word("@!","FETCHSTORE","0")
		*T++ = *S1++;
		NEXT;
	
	def_code_word("C@C!","FETCHSTOREBYTE","0")
		*(char *)T++ = *(char *)S1++;
		NEXT;
	
	def_code_word("!++","INCSTORE","0")
		*T += 1;
		POPD;
		NEXT;

	def_code_word("!--","DECSTORE","0")
		*T += 1;
		POPD;
		NEXT;

	def_code_word("C@","FETCHBYTE","0")
		T = *(char *)T;
		NEXT;

	def_code_word("C!","STOREBYTE","0")
		*(char *)T = (char)(*DSP++);
		POPD;
		NEXT;

	def_code_word("DUP+","DUPPLUS","0")
		PUSHD;
		S1 = S1 + 1;
		NEXT;

	def_code_word("OVER+","OVERPLUS","0")
		PUSHD;
		T = S2;
		S2 = S2 + 1;
		NEXT;

	def_code_word(",","COMMA","FL_IMMEDIATE")
		*(here++) = T;
		POPD;
		NEXT;

	def_code_word("DOCOL","DOCOL","FL_HIDDEN")
		*(RSP--) = IP;
		IP = (int ***)(W+1);
		NEXT;

	def_code_word("WORD","WORD","0")
		X = rb->w_pos;
		if ( X == 0 ) { //new word, so skip whitespaces
			while (1) {
				_key;
				if ( T == 0 ) NEXT;
				else if ( S1 > ' ') {POPD;break;}
				POPD;POPD;
			}
		}
		do { //copy chars from char buffer into word buffer until whitespace
			rb->w_buff[rb->w_pos++] = T;
			POPD;
			_key;
			if ( T == 0 ) NEXT;
			POPD;
		} while ( T > ' ' )
		T = rb->w_buff;PUSHD; //word address in word buffer
		T = rb->w_pos; //word length
		rb->w_pos = 0;
		NEXT;
			
	def_code_word("STRCMP","STRCMP","0") // ( str_addr1 str_addr2 len -- result )
		do {
			T = T - 1;
			if ( *(char *)S1[T] != *(char *)S2[T] ) {POPD;POPD;T=0;NEXT;}
		} while ( T > 0 )
		POPD;POPD;
		T = 1;
		NEXT;

	def_code_word("FIND","FIND","0")
		X=*latest;
		while (X != 0) {
			if ( strcmp( (const char*)T, (const char *)(X+2)) ) break;
			X = *X;
		}
		T = X;

	def_code_word("NUMBER","NUMBER","0")
		PUSHD; //string addr is S1
		T = 0; //val
		W = 1; //sign
		if ( (char)*T == '-' ) { W = -1; T += 1;}

		while ( *T != 0 ) {
			for ( X=0; X < base; X++ ) {
				if ( *T == digits[X] ) {
					T *= base;
					T += X;
					break;
				}
			}
			if ( X == base ) {
				T = 0;
				NEXT;
			}
		}
		W = W * T;
		T = S1; // T is non zero => number is valid
		S1 = W; // S1 is the number
		NEXT;

	def_code_word("KEY","KEY","0")
		IP = *(int ***)key;
		NEXT;

	def_code_word("EMIT","EMIT","0")
		IP = *(int ***)emit;
		NEXT;

	def_code_word("TELL","TELL","0");
		IP = *(int ***)tell;
		NEXT;

		


	def_code_word("TICK","'","FL_IMMEDIATE")

	

	def_code_word("switch_context","switch_context","0")

	   task_t *next_task;
	   int ms = get_ms();

	   task->dsp = DSP;
	   task->rsp = RSP;
	   
	   if (task->wake_at_ms < ms) task->wake_at_ms = ms;

	   next_task = first_task;

	   while (1) {
	      if (get_ms() > next_task->wake_at_ms || (next_task->semaphore != 0 && *next_task->semaphore != 0)) {
		 task = next_task;
		 DSP = task->dsp;
		 RSP = task->rsp;
	      }
	      else {
		 next_task = next_task->next_task;
	      }
	   }
	   NEXT;
	

	}
}
