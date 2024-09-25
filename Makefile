.POSIX:
.SUFFIXES:
CC = cc
LDLIBS =
LDFLAGS =
CFLAGS =
PREFIX = /usr/local

main: emulator.o
	$(CC) -Wall $(LDFLAGS) -o 8080em emulator.o $(LDLIBS)
emulator.o: emulator.c structs.h

dis: disassembler.o
	$(CC) -Wall $(LDFLAGS) -o 8080dis disassembler.o $(LDLIBS)
disassembler.o: disassembler.c

clean:
	rm -f 8080em 8080dis disassembler.o emulator.o

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
