#include <string.h>
#include "forth.h"
#include "forth_prims.h"
#include "registers.h"

extern int get_ms();
extern int uart_write(const void *src, int size);

#define _isdigit(C) for (X=base;X>=0;--X) {if (C == digits[X]) break;}

static char digits[]="0123456789ABCDEF";

#define _iskey (input_buffer->unread)
#define _readkey *(char *)(input_buffer->read_pos++);input_buffer->unread -= 1
//_key ( -- key is_key )
#define _key do { PUSHD;T=0;if (_iskey) {T=_readkey;PUSHD;T=1;} else PUSHD; } while (0)
      
asm(".equ FL_IMMEDIATE, 32");
asm(".equ FL_HIDDEN, 64");
DOCOL_LEN;

extern int *here;
extern int *latest;
int compiling;
int base=10;
int *key;
int *emit;
int *tell;

task_t *first_task;
task_t *task;

extern forth_input_buffer_t uart_input_buffer;
forth_input_buffer_t *input_buffer = &uart_input_buffer;

#define word_buf_len 32
char word_buf[word_buf_len];
int word_buf_pos;

#define def_word(name,label,flags) case __COUNTER__: \
   asm("\n"\
   ".global "label"\n"\
   ".global link_"label"\n"\
   ".align 4\n"\
   "link_"label":\n"\
   ".int link\n"\
   ".int 2f-1f+"flags"\n"\
   ".set link,link_"label"\n"\
   "1:\n"\
   ".ascii \""name"\"\n"\
   "2:\n"\
   ".align 4\n"\
   label":");

#define def_code_word(name,label,flags) def_word(name,label,flags);
#define def_forth_word(name,label,flags,def) def_word(name,label,flags) do {__label__ not_eliminate; asm goto(""::::not_eliminate); _DOCOL; not_eliminate: asm (def"\n.int EXIT"); } while (0);
#define _JZ(label) "JZ, "label" "
#define _JMP(label) "JMP, "label" "

void prims(int c) {
   asm(".set link,0");
   switch (c) {

      def_code_word("DOCOL","DOCOL","0")
         _DOCOL;
         break;

      def_code_word("EXIT","EXIT","0")
         IP = (int **) *RSP++;
         NEXT;
      
      def_code_word("BRK","BRK","0")
         NEXT;
      
      def_code_word("DUP","DUP","0")
         PUSHD;
         NEXT;
	
      def_code_word("RDUP","RDUP","0")
         PUSHD;
         T = (int)*RSP;
         NEXT;
	
      def_code_word("2DUP","DDUP","0")
         *(DSP-1) = T;
         *(DSP-2) = S1;
         DSP = DSP -2;
         NEXT;

      def_code_word("DUP+","DUPPLUS","0")
         PUSHD;
         S1 = S1 + 1;
         NEXT;

      def_code_word("OVER","OVER","0")
#define _over PUSHD; T = S2
         _over;
         NEXT;

      def_code_word("OVER+","OVERPLUS","0")
         _over;
         S2 = S2 + 1;
         NEXT;

      def_code_word("DROP","DROP","0")
         POPD;
         NEXT;

      def_code_word("2DROP","DDROP","0")
#define _DDROP T=*(DSP+1); DSP += 2
         _DDROP;
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

      def_code_word("-ROT","MINUSROT","0")
         X = T;
         T = S1;
         S1 = S2;
         S2 = X;
         NEXT;

      def_code_word("++","INC","0")
         T = T + 1;
         NEXT;

      def_code_word("--","DEC","0")
         T = T - 1;
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

      def_code_word("&","AND","0")
         T = T & *(DSP++);
         NEXT;

      def_code_word("|","OR","0")
         T = T | *(DSP++);
         NEXT;

      def_code_word("^","XOR","0")
         T = T ^ *(DSP++);
         NEXT;

      def_code_word("~","NOT","0")
         T = ~T;
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

      def_code_word("EXE","EXE","0");
         W=(int *) T;
         POPD;
         goto *W;

      def_code_word("JMP","JMP","0")
         IP = (int **)(*IP);
         NEXT;

      def_code_word("JZ","JZ","0")
         if (T) {
            IP++;
         }
         else {
            IP = (int **)(*IP);
         }
         POPD;
         NEXT;

      def_code_word(">R","TOR","0")
         *(--RSP) = (int *)T;
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
         *(int *)T++ = *(int *)S1++;
         NEXT;
      
      def_code_word("C@C!","FETCHSTOREBYTE","0")
         *(char *)T++ = *(char *)S1++;
         NEXT;
      
      def_code_word("!++","INCSTORE","0")
         *(int *)T += 1;
         POPD;
         NEXT;

      def_code_word("!--","DECSTORE","0")
         *(int *)T += 1;
         POPD;
         NEXT;

      def_code_word("C@","FETCHBYTE","0")
         T = *(char *)T;
         NEXT;

      def_code_word("C!","STOREBYTE","0")
         *(char *)T = (char)(*DSP++);
         POPD;
         NEXT;

      def_code_word(",","COMMA","0")
         *( (int *)here++) = T;
         POPD;
         NEXT;
      
      def_code_word("CELL","CELL","0");
         PUSHD;
         T = sizeof(int);
         NEXT;

      // WORD read new word from readbuffer ( -- addr len )
      def_code_word("WORD","WORD","0")
         // skip whitespaces
         for ( ; ; ) {
            _key;
            //_key ( -- key is_key )
            if ( T == 0 ) {
               NEXT;
            }
            else if ( S1 > ' ') {POPD;break;}
            POPD;POPD;
         }
         
        word_buf_pos = 0; 
         
        do { //copy chars from char buffer into word buffer until whitespace
            word_buf[word_buf_pos++] = T;
            POPD;
            _key;
            if ( T == 0  ) {POPD;break;}
            POPD;
         } while ( T > ' ' );
         T = (int)word_buf;PUSHD; //address of the word buffer
         T = word_buf_pos; //word length
         NEXT;

      // STRCMP compare strings ( str_addr1 len1 str_addr2 len2 -- result )     
      def_code_word("STRCMP","STRCMP","0")
         X = 0; //default result is false
         if ( T == S2 ) {
            POPD;
            X = 1;
            do {
               S1 = S1 - 1;
               if ( ((char *)T)[S1] != ((char *)S2)[S1]  ) {X=0;break;}
            } while ( S1 > 0 );
         }
         else {
            POPD;
         }
         
         POPD;POPD;
         T = X;
         NEXT;

      // WCMP compare string with dictionary item ( str_addr len dict_addr -- result ) 
      def_forth_word("WCMP","WCMP","FL_HIDDEN","\
         .int CELL, ADD, DUP, CELL, ADD, SWAP, FETCH, LIT, 31, AND, STRCMP");

      // FIND ( str_addr len -- dict_addr )
      def_forth_word("FIND","FIND","0","\
         .int LATEST\n\
         1: .int FETCH, DUP, "_JZ("2f")", TOR, DDUP, RDUP, WCMP, FROMR, SWAP, "_JZ("1b")"\n\
         2: .int MINUSROT, DDROP");

      // NUMBER ( addr len -- value is_valid_number )
      def_code_word("NUMBER","NUMBER","0")
         T = (int)S1 + T; // process until this address
         PUSHD; //S2 = str addr, S1 = len
         W = (int *)1; //sign
         T = 0; //initial value
         if ( *(char *)S2 == '-' ) { W = (int *)-1; S2 += 1;}

         while ( S2 < S1 ) {
            _isdigit( *(char *)S2 ); // X == [0..base-1] valid digit, X == -1 not valid digit
            if ( X == -1 ) {POPD; T=0; NEXT;}
            T = T * base;
            T = T + X;
            S2 += 1;
         }
         *(++DSP) = (int)W * T; // DROP, DROP, PUSHD
         T = 1;
         NEXT;

      def_forth_word("INTERPRET","INTERPRET","0","\
         1: .int WORD, DUP, "_JZ("7f")",\
         DDUP, FIND, DUP, "_JZ("3f")",\
          SWAP, DROP, SWAP, DROP, COMPILING, "_JZ("2f")",\
          BRK, DUP, CELL, ADD, FETCH, LIT, FL_IMMEDIATE, AND, "_JZ("4f")"\n\
         2: .int XT, EXE, OK, "_JMP("1b")"\n\
         3: .int DROP, NUMBER, "_JZ("6f")",\
          COMPILING, "_JZ("5f")", LIT, LIT, COMMA, COMMA\n\
            .int "_JMP("1b")"\n\
         4: .int XT, COMMA\n\
            .int "_JMP("1b")"\n\
         7: .int DDROP\n\
            .int "_JMP("1b")"\n\
         5: .int OK, "_JMP("1b")"\n\
         6: .int DROP, NOT_A_WORD\n\
            .int "_JMP("1b")"");


      def_code_word("KEY","KEY","0")
         _key;
         NEXT;

      def_code_word("EMIT","EMIT","0")
         PUSHD;
         uart_write( (const void *)DSP, 1);        
         POPD;
         POPD;
         NEXT;

      def_code_word("OK","OK","0")
         uart_write( " <ok>\n", 6);        
         NEXT;

      def_code_word("BASE","BASE","0")
         PUSHD
         T = base;        
         NEXT;

      def_code_word("DEC","DECIMAL","0")
         base = 10;        
         NEXT;

      def_code_word("HEX","HEX","0")
         base = 16;        
         NEXT;

      def_code_word(".","DOT","0") //( n -- )
         word_buf_pos = word_buf_len ;
         while (T) {
            word_buf_pos -= 1;
            X = T % base;
            word_buf[word_buf_pos] = digits[X];
            T = T / base;
         }
         uart_write( &word_buf[word_buf_pos], word_buf_len - word_buf_pos);
         POPD;
         NEXT;

      def_code_word("NOT_A_WORD","NOT_A_WORD","0")
         uart_write( (const void *)"NOT A WORD\n", 11);
         NEXT;

      def_code_word("TELL","TELL","0");
         uart_write( (const void *)S1, (int)T);        
         DSP++;
         POPD;
         NEXT;

      def_forth_word("ALIGN4","ALIGN4","0","\
         .int LIT, 3, ADD, LIT, 3, NOT, AND");

      def_forth_word(">XT","XT","0","\
         .int CELL, ADD, DUP, FETCH, LIT, 31, AND, ADD, CELL, ADD, ALIGN4");
         
      def_forth_word("'","TICK","FL_IMMEDIATE","\
         .int WORD, FIND, DUP, "_JZ("1f")" , \
         XT\n\
         1:");
      
      def_code_word("COMPILING","COMPILING","FL_IMMEDIATE")
         PUSHD;
         T = compiling;
         NEXT;

      def_code_word("HERE","HERE","0")
         PUSHD;
         T = (int)&here;
         NEXT;

      def_code_word("LATEST","LATEST","0")
         PUSHD;
         T = (int)&latest;
         NEXT;

      def_code_word("[","LBRAC","FL_IMMEDIATE")
         compiling=0;
         NEXT;

      def_code_word("]","RBRAC","FL_IMMEDIATE")
         compiling=1;
         NEXT;

         // ( src dst len -- )
      def_code_word("CCOPY","CCOPY","0")
         memcpy( (void *)S1, (const void*)S2,T);
         DSP +=2;
         POPD;
         NEXT;

         // ( word_addr len -- )
      def_forth_word("CREATE","CREATE","0","\
         .int WORD, HERE, FETCH, LATEST, FETCH, COMMA, LATEST, STORE, \
         DUP, COMMA, \
         SWAP, OVER, HERE, FETCH, SWAP, CCOPY, \
         HERE, FETCH, ADD, ALIGN4, \
         HERE, STORE, CPY_DOCOL");
         
      def_forth_word("HIDDEN","HIDDEN","0","\
         .int LATEST, FETCH, CELL, ADD, DUP, FETCH, LIT, FL_HIDDEN, XOR, SWAP, STORE");
      
      def_forth_word("CPY_DOCOL","CPY_DOCOL","FL_HIDDEN","\
         .int LIT, DOCOL, HERE, FETCH, LIT, docol_len, \
         CCOPY, \
         HERE, FETCH, LIT, docol_len, ADD, ALIGN4, \
         HERE, STORE");

      def_forth_word(":","COLON","0","\
         .int CREATE, \
         HIDDEN, \
         RBRAC");

      def_forth_word(";","SEMICOLON","FL_IMMEDIATE","\
         .int LIT, EXIT, COMMA, \
         HIDDEN, \
         LBRAC");

      def_forth_word("POSTPONE","POSTPONE","FL_IMMEDIATE","\
         .int WORD, FIND, XT, COMMA");

      def_forth_word("DEBUG","DEBUG","0","\
         .int BRK, DOT, \
         BRK");


      def_code_word("switch_context","switch_context","0")

         int ms = get_ms();
         PUSHD;
         task->dsp = DSP;
         task->rsp = RSP;
         
         if (task->wake_at_ms < ms) task->wake_at_ms = ms;

         X = (int)first_task;

         while (1) {
            if ( ms > ((task_t *)X)->wake_at_ms || (((task_t *)X)->semaphore != 0 && ( *((task_t *)X)->semaphore != 0) ) ) {
                task = ((task_t *)X);
                DSP = task->dsp;
                RSP = task->rsp;
               POPD;
            }
            else {
               X = (int)((task_t*)X)->next_task;
            }
         }
         NEXT;
      

   }
}

