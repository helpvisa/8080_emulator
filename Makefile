.POSIX:
.SUFFIXES:
CC = cc
LDLIBS =
LDFLAGS =
CFLAGS =
PREFIX = /usr/local

all: disassembler.o emulator.o
	$(CC) -Wall $(LDFLAGS) -o 8080em emulator.o disassembler.o $(LDLIBS)
emulator.o: emulator.c structs.h disassembler.h


dis: standalone_disassembler.o
	$(CC) -Wall $(LDFLAGS) -o 8080dis standalone_disassembler.o $(LDLIBS)
standalone_disassembler.o: standalone_disassembler.c disassembler.h
disassembler.o: disassembler.c disassembler.h

clean:
	rm -f 8080em 8080dis disassembler.o emulator.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
