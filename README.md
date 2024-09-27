# 8080 emulator

An educational exercise in programming an 8080 emulator in C.
Includes a disassembler for compiled 8080 code.


### Some notes on design...
At the moment the emulator is basically one massive switch case for opcodes.
There's a lot of repetition here; I was focused on initial functionality over functional programming (haha).
There's plenty of room to split things into functions and stop repeating myself;
my approach was to emulate until my test programs met an unimplemented instruction, then implement said instruction.
Every instruction requires only a single cycle at the moment, so things are not accurate.

As it stands, the C program included here is a pure emulation of the CPU and doesn't care at all about I/O.
You can run the emulator with the "0 1" args appended:

```
8080em PATH_TO_ROM 0 1
```

which will disable debug output and enable memory dumps (64 times / s) to a named pipe.

The python frontend is a GTK app that reads from this pipe at /tmp/8080fifo and spits out a grayscale image of the memory at the state it was read from the pipe.
The plan is to decouple cpu emulation from frontend emulation so arbitrary programs can be run.
Accordingly, a second named pipe can be used to send input from the frontend to the C backend, and then the emulator's output can be fed back again, etc.
I'm more focused on finding *any* working implementation rather than the perfect implementation at the moment.
This reduces portability and generally limits the project to Unix systems, but it's for learning after all!


### References
[This website was used extensively during the writing of this project.](http://emulator101.com/)
