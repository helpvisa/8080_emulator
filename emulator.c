#include "structs.h"
#include "disassembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
// portable method of clearing screens and sleeping
/* #ifdef _WIN32 */
/* #include <conio.h> */
/* #include <windows.h> */
/* #define usleep(double time) Sleep(time / 1000) */
/* #else */
#include <unistd.h>
#define clrscr() printf("\e[1;1H\e[2J")
/* #endif */

/* currently missing a way of setting the auxiliary carry flag
 * state->cc.ac will never change as such */

// hoist functions
void unimplemented_instruction(State *state);
uint8_t parity(int num, int bits);
void emulate(State *state);

// Global emulator params
int output = 0;
int use_socket = 0;
double clockrate = 0.0000005; // 1s / 2MHz = 5 * 10^-7
/* double clockrate = 16.666; // a 60 hertz clockrate for testing */


int main(int argc, char *argv[]) {
  char *filename = NULL;
  FILE *fptr;
  // set params (in a horrible way) and load ROM
  if (argc > 3) {
    use_socket = strtod(argv[3], NULL);
  }
  if (argc > 2) {
    output = strtod(argv[2], NULL);
  }
  if (argc > 1) {
    filename = argv[1];
  } else {
    printf("You need to specify a file.\n");
    return 1;
  }
  // init file pointer
  fptr = fopen(filename, "rb");
  if (fptr == NULL) {
    printf("File does not exist or could not be opened.");
    return 1;
  }
  // determine file size and allocate buffer
  fseek(fptr, 0L, SEEK_END);
  int fsize = ftell(fptr);
  /* printf("%d\n", fsize); */
  fseek(fptr, 0L, SEEK_SET);
  uint8_t *rom_buffer = malloc(fsize * sizeof(uint8_t));
  fread(rom_buffer, fsize, 1, fptr);
  // close the file; we have read it and are done with it
  fclose(fptr);

  // initialize emulator
  // create a 64kb memory buffer, zero it, and copy the rom into it
  uint8_t *memory_buffer = malloc(65535 * sizeof(uint8_t));
  for (int i = 0; i < 65535; i++) {
    memory_buffer[i] = 0x00;
  }
/* #ifdef CPUDIAG */
/*   memcpy(memory_buffer, rom_buffer + 0x100, fsize); // offset is necessary */
/*   // fix first instruction */
/*   // JMP 0x100 */
/*   rom_buffer[0] = 0xc3; */
/*   rom_buffer[1] = 0; */
/*   rom_buffer[2] = 0x01; */
/*   // fix stack pointer from 0x6ad to 0x7ad */
/*   // adds offset of 0x100 to sp */
/*   rom_buffer[368] = 0x7; */
/*   // skip DAA test since AC is not implemented at the moment */
/*   rom_buffer[0x59c] = 0xc3; // JMP */
/*   rom_buffer[0x59d] = 0xc2; */
/*   rom_buffer[0x59e] = 0x05; */
/* #else */
  memcpy(memory_buffer, rom_buffer, fsize);
/* #endif */
  struct ConditionCodes conditionals = {0,0,0,0,0,0};
  struct State machine_state = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0000, 0x0000,
    rom_buffer, conditionals,
    0x00
  };

  // run emulator
  while (1) {
    clrscr();
    emulate(&machine_state);
    /* getchar(); */
    usleep(clockrate * 1000);
  }

  // free memory and exit
  free(rom_buffer);
  return 0;
}


// determines if # of 1s is odd or even
uint8_t parity(int n, int bits) {
  uint8_t i = bits / 2;
  while (i >= 1) {
    n ^= n >> i;
    i /= 2;
  }
  return (~n) & 1;
}

// instruction functions
void unimplemented_instruction(State *state) {
  printf("Error: instruction not implemented!\n");
  disassemble(state->memory, state->pc, output);
/* #ifdef CPUDIAG */
/*   printf("In CPUDIAG mode.\n"); */
/* #endif */
  exit(1);
}

// we break CALL into its own function to better handle ifdefs and use it CNZ etc.
// this should eventually be done for all instructions... not very DRY...
void CALL(State *state, uint8_t *opcode) {
/* #ifdef CPUDIAG */
/*   if (5 == ((*(opcode + 2) << 8) | *(opcode + 1))) { */
/*     printf("Oh, we're trying to print!\n"); */
/*     if (state->c == 9) { */
/*       /\* uint16_t offset = (state->d << 8) | (state->e); *\/ */
/*       char *str = (state->memory); // using offset causes segfault... accept some garbage */
/*       while (*str != '$') { */
/*         printf("%c", *str++); */
/*       } */
/*       printf("\n"); */
/*     } else if (state->c == 2) { */
/*       printf("print char routine called\n"); */
/*     } */
/*   } else if (0 == ((*(opcode + 2) << 8) | *(opcode + 1))) { */
/*     exit(0); */
/*   } else */
/* #endif */
  /* { */
  // store return address
  uint16_t ret = state->pc + 2;
  state->memory[state->sp - 1] = (ret >> 8) & 0xff;
  state->memory[state->sp - 2] = (ret & 0xff);
  state->sp -= 2;
  // and jump to next address
  state->pc = (*(opcode + 2) << 8) | *(opcode + 1);
  state->pc -= 1;
  /* } */
}


void emulate(State *state) {
  unsigned char *opcode = &state->memory[state->pc];
  // print out the current instruction
  if (output) {
    disassemble(state->memory, state->pc, output);
  }

  switch (*opcode) {
    case 0x00: break; //NOP
    case 0x01: { // LXI B,D16
      state->c = opcode[1]; // c <- byte 2
      state->b = opcode[2]; // b <- byte 3
      state->pc += 2; // increment by 2 extra bytes (3 byte length)
    } break;
    case 0x02: { // STAX B
      // BC together create a 16 bit register (BC) in parentheses
      state->memory[((uint16_t)state->b << 8) | (state->c)] = state->a; // store A in (BC)
    } break;
    case 0x03: { // INX B
      uint16_t bc = (uint16_t)(state->b << 8) | (state->c);
      bc++;
      state->b = bc >> 8 & 0xff;
      state->c = bc & 0xff;
    } break;
    case 0x04: { // INR B
      uint16_t answer = state->b + 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->b = answer & 0xff;
    } break;
    case 0x05: { // DCR B
      uint16_t answer = (uint16_t)state->b - 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->b = answer & 0xff;
    } break;
    case 0x06: { // MVI B,D8
      state->b = opcode[1];
      state->pc++;
    } break;
    case 0x07: { // RLC
      uint8_t x = state->a;
      state->a = ((x & 1) << 7) | (x >> 1);
      state->cc.cy = (1 == (x & 1));
    } break;
    case 0x08: break; // NOP
    case 0x09: { // DAD B
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      uint16_t bc = (uint16_t)(state->b << 8) | (state->c);
      hl += bc;
      state->cc.cy = (hl > 0xff); // DAD affects only carry flag
      state->h = hl >> 8 & 0xff;
      state->l = hl & 0xff;
    } break;
    case 0x0a: { // LDAX B
      uint16_t bc = (uint16_t)(state->b << 8) | (state->c);
      state->a = state->memory[bc];
    } break;
    case 0x0b: { // DCX B
      uint16_t bc = (uint16_t)(state->b << 8) | (state->c);
      bc--;
      state->b = bc >> 8 & 0xff;
      state->c = bc & 0xff;
    } break;
    case 0x0c: { // INR C
      uint16_t answer = (uint16_t)state->c + 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->c = answer & 0xff;
    } break;
    case 0x0d: { // DCR C
      uint16_t answer = (uint16_t)state->c - 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->c = answer & 0xff;
    } break;
    case 0x0e: { // MVI C,D8
      state->c = opcode[1];
      state->pc++;
    } break;
    case 0x0f: { // RRC
      uint8_t x = state->a;
      state->a = ((x & 1) << 7) | (x >> 1);
      state->cc.cy = (1 == (x & 1));
    } break;
    case 0x10: break; // NOP
    case 0x11: { // LXI D,D16
      state->e = opcode[1];
      state->d = opcode[2];
      state->pc += 2;
    } break;
    case 0x12: { // STAX D
      uint16_t de = (uint16_t)(state->d << 8) | (state->e);
      state->memory[de] = state->a;
    } break;
    case 0x13: { // INX D
      uint16_t de = (uint16_t)(state->d << 8) | (state->e);
      de++;
      state->d = de >> 8 & 0xff;
      state->e = de & 0xff;
    } break;
    case 0x14: { // INR D
      uint16_t answer = state->d + 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->d = answer & 0xff;
    } break;
    case 0x15: { // DCR D
      uint16_t answer = state->d - 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->d = answer & 0xff;
    } break;
    case 0x16: { // MVI D,D8
      state->d = opcode[1];
      state->pc++;
    } break;
    case 0x17: { // RAL
      uint8_t x = state->a;
      state->a = (x << 1) | state->cc.cy;
      state->cc.cy = (1 == ((x >> 7) & 1));
    } break;
    case 0x18: break; // NOP
    case 0x19: { // DAD D
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      uint16_t de = (uint16_t)(state->d << 8) | (state->e);
      hl += de;
      state->cc.cy = (hl > 0xff); // DAD affects only carry flag
      state->h = hl >> 8 & 0xff;
      state->l = hl & 0xff;
    } break;
    case 0x1a: { // LDAX D
      uint16_t de = (uint16_t)(state->d << 8) | (state->e);
      state->a = state->memory[de];
    } break;
    case 0x1b: { // DCX D
      uint16_t de = (uint16_t)(state->d << 8) | (state->e);
      de--;
      state->d = de >> 8 & 0xff;
      state->e = de & 0xff;
    } break;
    case 0x1c: { // INR E
      uint16_t answer = state->e + 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->e = answer & 0xff;
    } break;
    case 0x1d: { // DCR E
      uint16_t answer = state->e - 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->e = answer & 0xff;
    } break;
    case 0x1e: { // MVI E,D8
      state->e = opcode[1];
      state->pc++;
    } break;
    case 0x1f: { // RAR
      uint8_t x = state->a;
      state->a = (state->cc.cy << 7) | (x >> 1);
      state->cc.cy = (1 == (x & 1));
    } break;
    case 0x20: break; // NOP
    case 0x21: { // LXI H,D16
      state->l = opcode[1];
      state->h = opcode[2];
      state->pc += 2;
    } break;
    case 0x22: { // SHLD adr
      uint16_t adr = (opcode[2] << 8) | opcode[1];
      state->memory[adr] = state->l;
      state->memory[adr + 1] = state->h;
      state->pc += 2;
    } break;
    case 0x23: { // INX H
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      hl++;
      state->h = hl >> 8 & 0xff;
      state->l = hl & 0xff;
    } break;
    case 0x24: { // INR H
      uint16_t answer = state->h + 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->h = answer & 0xff;
    } break;
    case 0x25: { // DCR H
      uint16_t answer = state->h - 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->h = answer & 0xff;
    } break;
    case 0x26: { // MVI H,D8
      state->h = opcode[1];
      state->pc++;
    } break;
    case 0x27: unimplemented_instruction(state); break; // DAA (unimplemented)
    case 0x28: break; // NOP
    case 0x29: { // DAD H
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      hl += hl;
      state->cc.cy = (hl > 0xff); // DAD affects only carry flag
      state->h = hl >> 8 & 0xff;
      state->l = hl & 0xff;
    } break;
    case 0x2a: { // LHLD adr
      uint16_t adr = (opcode[2] << 8) | opcode[1];
      state->l = state->memory[adr];
      state->h = state->memory[adr + 1];
      state->pc += 2;
    } break;
    case 0x2b: { // DCX H
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      hl--;
      state->h = hl >> 8 & 0xff;
      state->l = hl & 0xff;
    } break;
    case 0x2c: { // INR L
      uint16_t answer = state->l + 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->l = answer & 0xff;
    } break;
    case 0x2d: { // DCR L
      uint16_t answer = state->l - 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->l = answer & 0xff;
    } break;
    case 0x2e: { // MVI L,D8
      state->l = opcode[1];
      state->pc++;
    } break;
    case 0x2f: { // CMA (NOT mask)
      state->a = ~state->a;
      // flags are not affected
    } break;
    case 0x30: break; // NOP
    case 0x31: { // LXI SP,D16
      state->sp = opcode[1]; // c <- byte 2
      state->sp = opcode[2] << 8; // b <- byte 3
      state->pc += 2;
    } break;
    case 0x32: { // STA adr
      uint16_t adr = (opcode[2] << 8) | (opcode[1]);
      state->memory[adr] = state->a;
      state->pc += 2;
    } break;
    case 0x33: { // INX SP
      state->sp++;
    } break;
    case 0x34: { // INR M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      hl++;
      state->cc.z = ((hl & 0xff) == 0);
      state->cc.s = ((hl & 0x80) != 0);
      state->cc.p = parity(hl & 0xff, 8);
      state->h = hl >> 8 & 0xff;
      state->l = hl & 0xff;
    } break;
    case 0x35: { // DCR M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      hl--;
      state->cc.z = ((hl & 0xff) == 0);
      state->cc.s = ((hl & 0x80) != 0);
      state->cc.p = parity(hl & 0xff, 8);
      state->h = hl >> 8 & 0xff;
      state->l = hl & 0xff;
    } break;
    case 0x36: { // MVI M,D8
      uint16_t adr = (uint16_t)(state->h << 8) | (state->l);
      state->memory[adr] += opcode[1];
      state->pc++;
    } break;
    case 0x37: { // STC
      // set carry flag
      state->cc.cy = 1;
    } break;
    case 0x38: break; // NOP
    case 0x39: { // DAD SP
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      hl += state->sp;
      state->cc.cy = (hl > 0xff); // DAD affects only carry flag
      state->h = hl >> 8 & 0xff;
      state->l = hl & 0xff;
    } break;
    case 0x3a: { // LDA adr
      uint16_t adr = (opcode[2] << 8) | (opcode[1]);
      state->a = state->memory[adr];
      state->pc += 2;
    } break;
    case 0x3b: { // DCX SP
      state->sp--;
    } break;
    case 0x3c: { // INR A
      uint16_t answer = state->a += 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x3d: { // DCR A
      uint16_t answer = state->a -= 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x3e: { // MVI A,D8
      state->a = opcode[1];
      state->pc++;
    } break;
    case 0x3f: { // CMC
      // flip our carry flag
      state->cc.cy = ~state->cc.cy;
    } break;
    case 0x40: { // MOV B,B
      state->b = state->b;
    } break;
    case 0x41: { // MOV B,C
      state->b = state->c;
    } break;
    case 0x42: { // MOV B,D
      state->b = state->d;
    } break;
    case 0x43: { // MOV B,E
      state->b = state->e;
    } break;
    case 0x44: { // MOV B,H
      state->b = state->h;
    } break;
    case 0x45: { // MOV B,L
      state->b = state->l;
    } break;
    case 0x46: { // MOV B,M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->b = state->memory[hl];
    } break;
    case 0x47: { // MOV B,A
      state->b = state->a;
    } break;
    case 0x48: { // MOV C,B
      state->c = state->b;
    } break;
    case 0x49: { // MOV C,C
      state->c = state->c;
    } break;
    case 0x4a: { // MOV C,D
      state->c = state->d;
    } break;
    case 0x4b: { // MOV C,E
      state->c = state->e;
    } break;
    case 0x4c: { // MOV C,H
      state->c = state->h;
    } break;
    case 0x4d: { // MOV C,L
      state->c = state->l;
    } break;
    case 0x4e: {
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->c = state->memory[hl];
    } break;
    case 0x4f: {
      state->c = state->a;
    } break;
    case 0x50: { // MOV D,B
      state->d = state->b;
    } break;
    case 0x51: { // MOV D,C
      state->d = state->c;
    } break;
    case 0x52: { // MOV D,D
      state->d = state->d;
    } break;
    case 0x53: { // MOV D,E
      state->d = state->e;
    } break;
    case 0x54: { // MOV D,H
      state->d = state->h;
    } break;
    case 0x55: { // MOV D,L
      state->d = state->l;
    } break;
    case 0x56: { // MOV D,M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->d = state->memory[hl];
    } break;
    case 0x57: { // MOV D,A
      state->d = state->a;
    } break;
    case 0x58: { // MOV E,B
      state->e = state->b;
    } break;
    case 0x59: { // MOV E,C
      state->e = state->c;
    } break;
    case 0x5a: { // MOV E,D
      state->e = state->d;
    } break;
    case 0x5b: { // MOV E,E
      state->e = state->e;
    } break;
    case 0x5c: { // MOV E,H
      state->e = state->h;
    } break;
    case 0x5d: { // MOV E,L
      state->e = state->l;
    } break;
    case 0x5e: { // MOV E,M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->e = state->memory[hl];
    } break;
    case 0x5f: { // MOV E,A
      state->e = state->a;
    } break;
    case 0x60: { // MOV H,B
      state->h = state->b;
    } break;
    case 0x61: { // MOV H,C
      state->h = state->c;
    } break;
    case 0x62: { // MOV H,D
      state->h = state->d;
    } break;
    case 0x63: { // MOV H,E
      state->h = state->e;
    } break;
    case 0x64: { // MOV H,H
      state->h = state->h;
    } break;
    case 0x65: { // MOV H,L
      state->h = state->l;
    } break;
    case 0x66: { // MOV H,M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->h = state->memory[hl];
    } break;
    case 0x67: { // MOV H,A
      state->h = state->a;
    } break;
    case 0x68: { // MOV L,B
      state->l = state->b;
    } break;
    case 0x69: { // MOV L,C
      state->l = state->c;
    } break;
    case 0x6a: { // MOV L,D
      state->l = state->d;
    } break;
    case 0x6b: { // MOV L,E
      state->l = state->e;
    } break;
    case 0x6c: { // MOV L,H
      state->l = state->h;
    } break;
    case 0x6d: { // MOV L,L
      state->l = state->l;
    } break;
    case 0x6e: { // MOV L,<
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->l = state->memory[hl];
    } break;
    case 0x6f: { // MOV L,A
      state->l = state->a;
    } break;
    case 0x70: { // MOV M,B
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->memory[hl] = state->b;
    } break;
    case 0x71: { // MOV M,C
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->memory[hl] = state->c;
    } break;
    case 0x72: { // MOV M,D
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->memory[hl] = state->d;
    } break;
    case 0x73: { // MOV M,E
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->memory[hl] = state->e;
    } break;
    case 0x74: { // MOV M,H
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->memory[hl] = state->h;
    } break;
    case 0x75: { // MOV M,L
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->memory[hl] = state->l;
    } break;
    case 0x76: { // HLT
      exit(0); // program halted and so we happily halt ours too
    } break;
    case 0x77: { // MOV M,A
      uint16_t adr = (uint16_t)(state->h << 8) | (state->l);
      state->memory[adr] = state->a;
    } break;
    case 0x78: { // MOV A,B
      state->a = state->b;
    } break;
    case 0x79: { // MOV A,C
      state->a = state->c;
    } break;
    case 0x7a: { // MOV A,D
      state->a = state->d;
    } break;
    case 0x7b: { // MOV A,E
      state->a = state->e;
    } break;
    case 0x7c: { // MOV A,H
      state->a = state->h;
    } break;
    case 0x7d: { // MOV A,L
      state->a = state->l;
    } break;
    case 0x7e: { // MOV A,M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->a = state->memory[hl];
    } break;
    case 0x7f: { // MOV A,A
      state->a = state->a;
    } break;
    case 0x80: { // ADD B
      // demonstration code: 0x81 shows condensed version
      // math done in higher precision in order to capture carry
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->b;
      // if result is zero, set zero flag
      // we AND with 0xff to reduce to uint8_t
      if ((answer & 0xff) == 0) {
        state->cc.z = 1;
      } else {
        state->cc.z = 0;
      }
      // if bit 7 is set, set the sign flag, else clear
      if (answer & 0x80) {
        state->cc.s = 1;
      } else {
        state->cc.s = 0;
      }
      // set carry flag
      if (answer > 0xff) {
        state->cc.cy = 1;
      } else {
        state->cc.cy = 0;
      }
      // test parity and set flag appropriately
      state->cc.p = parity(answer & 0xff, 8);
      // now store in A
      state->a = answer & 0xff;
    } break;
    case 0x81: { // ADD C
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->c;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x82: { // ADD D
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->d;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x83: { // ADD E
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->e;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x84: { // ADD H
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->h;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x85: { // ADD L
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->l;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x86: { // ADD M
      // copy from memory into accumulator
      uint16_t offset = (state->h << 8) | (state->l);
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->memory[offset];
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x87: { // ADD A
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->a;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x88: { // ADC B
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->b + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x89: { // ADC C
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->c + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x8a: { // ADC D
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->d + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x8b: { // ADC E
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->e + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x8c: { // ADC H
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->h + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x8d: { // ADC L
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->l + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x8e: { // ADC M
      // copy from memory into accumulator
      uint16_t offset = (state->h << 8) | (state->l);
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->memory[offset] + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x8f: { // ADC A
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->a + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x90: { // SUB B
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->b;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x91: { // SUB C
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->c;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x92: { // SUB D
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->d;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x93: { // SUB E
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->e;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x94: { // SUB H
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->h;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x95: { // SUB L
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->l;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x96: { // SUB M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->memory[hl];
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x97: { // SUB A
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->a;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x98: { // SBB B
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->b - state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x99: { // SBB C
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->c - state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x9a: { // SBB D
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->d - state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x9b: { // SBB E
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->e - state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x9c: { // SBB H
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->h - state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x9d: { // SBB L
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->l - state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x9e: { // SBB M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->memory[hl] - state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0x9f: { // SBB A
      uint16_t answer = (uint16_t)state->a - (uint16_t)state->a - state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xa0: { // ANA B
      uint16_t answer = (uint16_t)(state->a) & (state->b);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xa1: { // ANA C
      uint16_t answer = (uint16_t)(state->a) & (state->c);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xa2: { // ANA D
      uint16_t answer = (uint16_t)(state->a) & (state->d);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xa3: { // ANA E
      uint16_t answer = (uint16_t)(state->a) & (state->e);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xa4: { // ANA H
      uint16_t answer = (uint16_t)(state->a) & (state->h);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xa5: { // ANA L
      uint16_t answer = (uint16_t)(state->a) & (state->l);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xa6: { // ANA M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      uint16_t answer = (uint16_t)(state->a) & (state->memory[hl]);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xa7: { // ANA A
      uint16_t answer = (uint16_t)(state->a) & (state->a);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xa8: { // XRA B
      uint16_t answer = (uint16_t)(state->a) ^ (state->b);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xa9: { // XRA C
      uint16_t answer = (uint16_t)(state->a) ^ (state->c);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xaa: { // XRA D
      uint16_t answer = (uint16_t)(state->a) ^ (state->d);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xab:  { // XRA E
      uint16_t answer = (uint16_t)(state->a) ^ (state->e);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xac: { // XRA H
      uint16_t answer = (uint16_t)(state->a) ^ (state->h);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xad: { // XRA L
      uint16_t answer = (uint16_t)(state->a) ^ (state->l);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xae: { // XRA M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      uint16_t answer = (uint16_t)(state->a) ^ (state->memory[hl]);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xaf: { // XRA A
      uint16_t answer = (uint16_t)(state->a) ^ (state->a);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xb0: { // ORA B
      uint16_t answer = (uint16_t)(state->a) | (state->b);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xb1: { // ORA C
      uint16_t answer = (uint16_t)(state->a) | (state->c);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xb2: { // ORA D
      uint16_t answer = (uint16_t)(state->a) | (state->d);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xb3: { // ORA E
      uint16_t answer = (uint16_t)(state->a) | (state->e);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xb4: { // ORA H
      uint16_t answer = (uint16_t)(state->a) | (state->h);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xb5: { // ORA L
      uint16_t answer = (uint16_t)(state->a) | (state->l);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xb6: { // ORA M
      uint16_t adr = (uint16_t)(state->h << 8) | (state->l);
      uint16_t answer = (uint16_t)(state->a) & (state->memory[adr]);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xb7: { // ORA A
      uint16_t answer = (uint16_t)(state->a) | (state->a);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xb8: { // CMP B
      uint16_t answer = (uint16_t)(state->a) - (state->b);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
    } break;
    case 0xb9: { // CMP C
      uint16_t answer = (uint16_t)(state->a) - (state->c);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
    } break;
    case 0xba: { // CMP D
      uint16_t answer = (uint16_t)(state->a) - (state->d);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
    } break;
    case 0xbb: { // CMP E
      uint16_t answer = (uint16_t)(state->a) - (state->e);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
    } break;
    case 0xbc: { // CMP H
      uint16_t answer = (uint16_t)(state->a) - (state->h);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
    } break;
    case 0xbd: { // CMP L
      uint16_t answer = (uint16_t)(state->a) - (state->l);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
    } break;
    case 0xbe: { // CMP M
      uint16_t adr = (uint16_t)(state->h << 8) | (state->l);
      uint16_t answer = (uint16_t)(state->a) - (state->memory[adr]);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
    } break;
    case 0xbf: { // CMP A
      uint16_t answer = (uint16_t)(state->a) - (state->a);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
    } break;
    case 0xc0: { // RNZ
      if (0 == state->cc.z) {
        state->pc = state->memory[state->sp] | (state->memory[state->sp + 1] << 8);
        state->pc -= 1;
        state->sp += 2;
      }
    } break;
    case 0xc1: { // POP B
      state->c = state->memory[state->sp];
      state->b = state->memory[state->sp + 1];
      state->sp += 2;
    } break;
    case 0xc2: { // JNZ adr
      if (0 == state->cc.z) {
        state->pc = (opcode[2] << 8) | (opcode[1]);
        state->pc -= 1;
      } else {
        state->pc += 2;
      }
    } break;
    case 0xc3: { // JMP adr
      state->pc = (opcode[2] << 8) | (opcode[1]);
      state->pc -= 1; // decrement since we know we add at the end of the switch case
    } break;
    case 0xc4: // CNZ adr
      if (0 == state->cc.z) {
        CALL(state, opcode);
      } else {
        state->pc += 2;
      }
      break;
    case 0xc5: { // PUSH B
      state->memory[state->sp - 2] = state->c;
      state->memory[state->sp - 1] = state->b;
      state->sp -= 2;
    }
    case 0xc6: { // ADI D8
      // we add immediate, so opcode[1] fetches the next byte
      uint16_t answer = (uint16_t)state->a + (uint16_t)opcode[1];
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
      state->pc += 1;
    } break;
    case 0xc7: { // RST 0
      uint8_t adr[3] = {0x00, 0x00, 0x00};
      CALL(state, adr);
    } break;
    case 0xc8: { // RZ
      // if zero, RET
      if (state->cc.z) {
        state->pc = state->memory[state->sp] | (state->memory[state->sp + 1] << 8);
        state->pc -= 1;
        state->sp += 2;
      }
    } break;
    case 0xc9: { // RET
      // jump to previously store return address
      state->pc = state->memory[state->sp] | (state->memory[state->sp + 1] << 8);
      state->pc -= 1;
      state->sp += 2;
    } break;
    case 0xca: { // JZ adr
      if (state->cc.z) {
        state->pc = (opcode[2] << 8) | (opcode[1]);
        state->pc -= 1;
      }
    } break;
    case 0xcb: break; // NOP
    case 0xcc: { // CZ adr
      if (state->cc.z) {
        CALL(state, opcode);
      } else {
        state->pc += 2;
      }
    } break;
    case 0xcd: // CALL adr
      CALL(state, opcode);
      break;
    case 0xce: { // ACI D8
      uint16_t answer = (uint16_t)state->a + (uint16_t)opcode[1] + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer;
      state->pc++;
    } break;
    case 0xcf: { // RST 1
      uint8_t adr[3] = {0x00, 0x08, 0x00};
      CALL(state, adr);
    } break;
    case 0xd0: { // RNC
      if (0 == state->cc.cy) {
        state->pc = state->memory[state->sp] | (state->memory[state->sp + 1] << 8);
        state->pc -= 1;
        state->sp += 2;
      }
    } break;
    case 0xd1: { // POP D
      state->e = state->memory[state->sp];
      state->d = state->memory[state->sp + 1];
      state->sp += 2;
    } break;
    case 0xd2: unimplemented_instruction(state); break;
    case 0xd3: { // OUT D8
      ; // do nothing for now
    } break;
    case 0xd4: unimplemented_instruction(state); break;
    case 0xd5: { // PUSH D
      state->memory[state->sp - 2] = state->e;
      state->memory[state->sp - 1] = state->d;
      state->sp -= 2;
    } break;
    case 0xd6: unimplemented_instruction(state); break;
    case 0xd7: unimplemented_instruction(state); break;
    case 0xd8: unimplemented_instruction(state); break;
    case 0xd9: unimplemented_instruction(state); break;
    case 0xda: {
      if (state->cc.cy) {
        uint16_t adr = (opcode[2] << 8) | (opcode[1]);
        state->pc = adr;
        state->pc -= 1;
      } else {
        state->pc += 2;
      }
    } break;
    case 0xdb: unimplemented_instruction(state); break;
    case 0xdc: unimplemented_instruction(state); break;
    case 0xdd: unimplemented_instruction(state); break;
    case 0xde: unimplemented_instruction(state); break;
    case 0xdf: unimplemented_instruction(state); break;
    case 0xe0: unimplemented_instruction(state); break;
    case 0xe1: { // POP H
      state->l = state->memory[state->sp];
      state->h = state->memory[state->sp + 1];
      state->sp += 2;
    } break;
    case 0xe2: unimplemented_instruction(state); break;
    case 0xe3: unimplemented_instruction(state); break;
    case 0xe4: unimplemented_instruction(state); break;
    case 0xe5: { // PUSH H
      state->memory[state->sp - 2] = state->l;
      state->memory[state->sp - 1] = state->h;
      state->sp -= 2;
    } break;
    case 0xe6: { // ANI byte
      uint8_t x = state->a & opcode[1];
      state->cc.z = (x == 0);
      state->cc.s = (0x80 == (x & 0x80));
      state->cc.p = parity(x, 8);
      state->cc.cy = 0; // ANI clears CY
      state->a = x;
      state->pc++; // we used next byte, so increment
    } break;
    case 0xe7: unimplemented_instruction(state); break;
    case 0xe8: unimplemented_instruction(state); break;
    case 0xe9: unimplemented_instruction(state); break;
    case 0xea: unimplemented_instruction(state); break;
    case 0xeb: { // XCHG
      uint8_t old_h = state->h;
      uint8_t old_l = state->h;
      state->h = state->d;
      state->d = old_h;
      state->l = state->e;
      state->e = old_l;
    } break;
    case 0xec: unimplemented_instruction(state); break;
    case 0xed: unimplemented_instruction(state); break;
    case 0xee: unimplemented_instruction(state); break;
    case 0xef: unimplemented_instruction(state); break;
    case 0xf0: unimplemented_instruction(state); break;
    case 0xf1: { // POP PSW
      uint8_t psw = state->memory[state->sp];
      state->cc.z = (0x01 == (psw & 0x01));
      state->cc.s = (0x02 == (psw & 0x02));
      state->cc.p = (0x04 == (psw & 0x04));
      state->cc.cy = (0x05 == (psw & 0x08));
      state->cc.ac = (0x10 == (psw & 0x10));
      state->a = state->memory[state->sp + 1];
      state->sp += 2;
    } break;
    case 0xf2: unimplemented_instruction(state); break;
    case 0xf3: { // DI (disable interrupt)
      state->int_enable = 0;
    } break;
    case 0xf4: unimplemented_instruction(state); break;
    case 0xf5: { // PUSH PSW
      uint8_t psw = (
        state->cc.z |
        state->cc.s << 1 |
        state->cc.p << 2 |
        state->cc.cy << 3 |
        state->cc.ac << 4
      );
      state->memory[state->sp - 2] = psw;
      state->memory[state->sp - 1] = state->a;
      state->sp -= 2;
    } break;
    case 0xf6: unimplemented_instruction(state); break;
    case 0xf7: unimplemented_instruction(state); break;
    case 0xf8: unimplemented_instruction(state); break;
    case 0xf9: unimplemented_instruction(state); break;
    case 0xfa: { // JM adr
      // jump on minus; if sign flag is set we jump to adr
      if (state->cc.s) {
        uint16_t adr = (opcode[2] << 8) | opcode[1];
        state->pc = adr;
        state->pc -= 1;
      } else {
        state->pc += 2;
      }
    } break;
    case 0xfb: { // EI (enable interrupt)
      state->int_enable = 1;
    } break;
    case 0xfc: unimplemented_instruction(state); break;
    case 0xfd: unimplemented_instruction(state); break;
    case 0xfe: { // CPI byte
      uint8_t x = state->a - opcode[1];
      state->cc.z = (x == 0);
      state->cc.s = (0x80 == (x & 0x80));
      state->cc.p = parity(x, 8);
      state->cc.cy = (state-> a < opcode[1]);
      state->pc++; // increment extra because we used next byte
    } break;
    case 0xff: unimplemented_instruction(state); break;
  }

  // increment memory pointer
  state->pc += 1;

  // print cpu state
  if (output) {
    printf("\tC=\e[35m%d\e[0m, P=\e[35m%d\e[0m, S=\e[35m%d\e[0m, Z=\e[35m%d\e[0m\n", state->cc.cy, state->cc.p, state->cc.s, state->cc.z);
    printf(
      "\tA \e[33m$%02x\e[0m\tB \e[33m$%02x\e[0m\tC \e[33m$%02x\e[0m\t"
      "D \e[33m$%02x\e[0m\tE \e[33m$%02x\e[0m\tH \e[33m$%02x\e[0m\t"
      "L \e[33m$%02x\e[0m\tSP \e[35m%04x\e[0m\n",
      state->a, state->b, state->c, state->d, state->e, state->h, state->l, state->sp
    );
  }
  /* } else { */
  /*   printf("\tC=%d, P=%d, S=%d, Z=%d\n", state->cc.cy, state->cc.p, state->cc.s, state->cc.z); */
  /*   printf( */
  /*     "\tA $%02x\tB $%02x\tC $%02x\tD $%02x\tE $%02x\tH $%02x\tL $%02x\tSP %04x\n", */
  /*     state->a, state->b, state->c, state->d, state->e, state->h, state->l, state->sp */
  /*   ); */
  /* } */
}
