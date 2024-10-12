#!/bin/sh
xtensa-esp32-elf-gcc -Os -fverbose-asm -fno-crossjumping -c forth_docol.c -o forth_docol.o
xtensa-esp32-elf-gcc -Os -fverbose-asm -fno-crossjumping -S forth_prims.c
