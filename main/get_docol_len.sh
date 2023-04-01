#!/bin/sh
xtensa-esp32-elf-nm forth_docol.o|sed -n 's/^\([0-9A-F]\+\)...docol_len/#define docol_len 0x\1/p' > docol.h
