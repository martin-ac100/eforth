#!/bin/sh
xtensa-esp32-elf-gcc -Os -fno-crossjumping -c forth_docol.c -o forth_docol.o
xtensa-esp32-elf-nm forth_docol.o|sed -n 's/^\([0-9A-F]\+\)...docol_len/#define docol_len 0x\1/p' > docol.h
xtensa-esp32-elf-nm forth_docol.o|sed -n 's/^\([0-9A-F]\+\)...docol_len/#define DOCOL_LEN asm(".equ docol_len, 0x\1")/p' >> docol.h
xtensa-esp32-elf-gcc -Os -fverbose-asm -fno-crossjumping -S forth_prims.c
