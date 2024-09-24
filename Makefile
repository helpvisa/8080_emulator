.POSIX:
.SUFFIXES:
.PHONY: disassembler.o
CC = cc
LDLIBS =
LDFLAGS =
CFLAGS =
PREFIX = /usr/local

disassembler: disassembler.o
	$(CC) -Wall $(LDFLAGS) -o ./disassemble ./disassembler.o
disassembler.o:
	$(CC) -Wall $(CFLAGS) -c disassembler.c
