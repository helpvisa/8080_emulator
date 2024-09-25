#include "structs.h"
#include "disassembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
// portable method of clearing screens and sleeping
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#define usleep(double time) Sleep(time / 1000)
#else
#include <unistd.h>
#define clrscr() printf("\e[1;1H\e[2J")
#endif

/* currently missing a way of setting the auxiliary carry flag
 * state->cc.ac will never change as such */

void unimplemented_instruction(State *state);
uint8_t parity(int num, int bits);
void emulate(State *state);

int output_in_colour = 0;
int show_memory = 0;
double clockrate = 0.0000005; // 1s / 2MHz = 5 * 10^-7
/* double clockrate = 16.666; // a 60 hertz clockrate for testing */

int main(int argc, char *argv[]) {
  char *filename = NULL;
  FILE *fptr;
  // set params (in a horrible way) and load ROM
  if (argc > 3) {
    show_memory = strtod(argv[3], NULL);
  }
  if (argc > 2) {
    output_in_colour = strtod(argv[2], NULL);
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
  printf("%d\n", fsize);
  fseek(fptr, 0L, SEEK_SET);
  uint8_t *rom_buffer = malloc(fsize * sizeof(uint8_t));
  fread(rom_buffer, fsize, 1, fptr);
  // close the file; we have read it and are done with it
  fclose(fptr);

  // initialize emulator
  // create a 64kb memory buffer, zero it, and copy the rom into it
  uint8_t *memory_buffer = malloc(65536 * sizeof(uint8_t));
  for (int i = 0; i < 65536; i++) {
    memory_buffer[i] = 0x00;
  }
#ifdef CPUDIAG
  memcpy(memory_buffer, rom_buffer + 0x100, fsize); // offset is necessary
  // fix first instruction
  // JMP 0x100
  rom_buffer[0] = 0xc3;
  rom_buffer[1] = 0;
  rom_buffer[2] = 0x01;
  // fix stack pointer from 0x6ad to 0x7ad
  // adds offset of 0x100 to sp
  rom_buffer[368] = 0x7;
  // skip DAA test since AC is not implemented at the moment
  rom_buffer[0x59c] = 0xc3; // JMP
  rom_buffer[0x59d] = 0xc2;
  rom_buffer[0x59e] = 0x05;
#else
  memcpy(memory_buffer, rom_buffer, fsize);
#endif
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


// instruction functions
void unimplemented_instruction(State *state) {
  printf("Error: instruction not implemented!\n");
#ifdef CPUDIAG
  printf("In CPUDIAG mode.\n");
#endif
  exit(1);
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

void emulate(State *state) {
  unsigned char *opcode = &state->memory[state->pc];
  // print out the current instruction
  disassemble(state->memory, state->pc, output_in_colour);

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
    case 0x04: unimplemented_instruction(state); break;
    case 0x05: { // DCR B
      uint16_t answer = (uint16_t)state->b - 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->b = answer & 0xff;
    } break;
    case 0x06: { // MVI B,D8
      state->b = opcode[1];
      state->pc++;
    } break;
    case 0x07: unimplemented_instruction(state); break;
    case 0x08: unimplemented_instruction(state); break;
    case 0x09: unimplemented_instruction(state); break;
    case 0x0a: unimplemented_instruction(state); break;
    case 0x0b: unimplemented_instruction(state); break;
    case 0x0c: unimplemented_instruction(state); break;
    case 0x0d: { // DCR C
      uint16_t answer = (uint16_t)state->c - 1;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
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
    case 0x10: unimplemented_instruction(state); break;
    case 0x11: { // LXI D,D16
      state->e = opcode[1];
      state->d = opcode[2];
      state->pc += 2;
    } break;
    case 0x12: unimplemented_instruction(state); break;
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
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->d = answer & 0xff;
    } break;
    case 0x15: unimplemented_instruction(state); break;
    case 0x16: unimplemented_instruction(state); break;
    case 0x17: unimplemented_instruction(state); break;
    case 0x18: unimplemented_instruction(state); break;
    case 0x19: { // DAD D
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      uint16_t de = (uint16_t)(state->d << 8) | (state->e);
      hl += de;
      state->cc.cy = (hl > 0xff); // DAD affects only carry flag
      state->h = hl >> 8 & 0xff;
      state->l = hl & 0xff;
    } break;
    case 0x1a: { // LDAX D
      uint16_t adr = (uint16_t)(state->d << 8) | (state->e);
      state->a = state->memory[adr];
    } break;
    case 0x1b: unimplemented_instruction(state); break;
    case 0x1c: unimplemented_instruction(state); break;
    case 0x1d: unimplemented_instruction(state); break;
    case 0x1e: unimplemented_instruction(state); break;
    case 0x1f: { // RAR
      uint8_t x = state->a;
      state->a = (state->cc.cy << 7) | (x >> 1);
      state->cc.cy = (1 == (x & 1));
    } break;
    case 0x20: unimplemented_instruction(state); break;
    case 0x21: { // LXI H,D16
      state->l = opcode[1];
      state->h = opcode[2];
      state->pc += 2;
    } break;
    case 0x22: unimplemented_instruction(state); break;
    case 0x23: { // INX H
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      hl++;
      state->h = hl >> 8 & 0xff;
      state->l = hl & 0xff;
    } break;
    case 0x24: unimplemented_instruction(state); break;
    case 0x25: unimplemented_instruction(state); break;
    case 0x26: { // MVI H,D8
      state->h = opcode[1];
      state->pc++;
    } break;
    case 0x27: unimplemented_instruction(state); break;
    case 0x28: unimplemented_instruction(state); break;
    case 0x29: { // DAD H
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      hl += hl;
      state->cc.cy = (hl > 0xff); // DAD affects only carry flag
      state->h = hl >> 8 & 0xff;
      state->l = hl & 0xff;
    } break;
    case 0x2a: unimplemented_instruction(state); break;
    case 0x2b: unimplemented_instruction(state); break;
    case 0x2c: unimplemented_instruction(state); break;
    case 0x2d: unimplemented_instruction(state); break;
    case 0x2e: unimplemented_instruction(state); break;
    case 0x2f: { // CMA (not)
      state->a = ~state->a;
      // flags are not affected
    } break;
    case 0x30: unimplemented_instruction(state); break;
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
    case 0x33: unimplemented_instruction(state); break;
    case 0x34: unimplemented_instruction(state); break;
    case 0x35: unimplemented_instruction(state); break;
    case 0x36: { // MVI M,D8
      uint16_t adr = (uint16_t)(state->h << 8) | (state->l);
      state->memory[adr] += opcode[1];
      state->pc++;
    } break;
    case 0x37: unimplemented_instruction(state); break;
    case 0x38: unimplemented_instruction(state); break;
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
    case 0x3b: unimplemented_instruction(state); break;
    case 0x3c: unimplemented_instruction(state); break;
    case 0x3d: unimplemented_instruction(state); break;
    case 0x3e: { // MVI A,D8
      state->a = opcode[1];
      state->pc++;
    } break;
    case 0x3f: unimplemented_instruction(state); break;
    case 0x40: unimplemented_instruction(state); break;
    case 0x41: unimplemented_instruction(state); break;
    case 0x42: unimplemented_instruction(state); break;
    case 0x43: unimplemented_instruction(state); break;
    case 0x44: unimplemented_instruction(state); break;
    case 0x45: unimplemented_instruction(state); break;
    case 0x46: unimplemented_instruction(state); break;
    case 0x47: unimplemented_instruction(state); break;
    case 0x48: unimplemented_instruction(state); break;
    case 0x49: unimplemented_instruction(state); break;
    case 0x4a: unimplemented_instruction(state); break;
    case 0x4b: unimplemented_instruction(state); break;
    case 0x4c: unimplemented_instruction(state); break;
    case 0x4d: unimplemented_instruction(state); break;
    case 0x4e: unimplemented_instruction(state); break;
    case 0x4f: unimplemented_instruction(state); break;
    case 0x50: unimplemented_instruction(state); break;
    case 0x51: unimplemented_instruction(state); break;
    case 0x52: unimplemented_instruction(state); break;
    case 0x53: unimplemented_instruction(state); break;
    case 0x54: unimplemented_instruction(state); break;
    case 0x55: unimplemented_instruction(state); break;
    case 0x56: { // MOV D,M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->d = state->memory[hl];
    } break;
    case 0x57: unimplemented_instruction(state); break;
    case 0x58: unimplemented_instruction(state); break;
    case 0x59: unimplemented_instruction(state); break;
    case 0x5a: unimplemented_instruction(state); break;
    case 0x5b: unimplemented_instruction(state); break;
    case 0x5c: unimplemented_instruction(state); break;
    case 0x5d: unimplemented_instruction(state); break;
    case 0x5e: { // MOV E,M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->e = state->memory[hl];
    } break;
    case 0x5f: unimplemented_instruction(state); break;
    case 0x60: unimplemented_instruction(state); break;
    case 0x61: unimplemented_instruction(state); break;
    case 0x62: unimplemented_instruction(state); break;
    case 0x63: unimplemented_instruction(state); break;
    case 0x64: unimplemented_instruction(state); break;
    case 0x65: unimplemented_instruction(state); break;
    case 0x66: { // MOV H,M
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->h = state->memory[hl];
    } break;
    case 0x67: unimplemented_instruction(state); break;
    case 0x68: unimplemented_instruction(state); break;
    case 0x69: unimplemented_instruction(state); break;
    case 0x6a: unimplemented_instruction(state); break;
    case 0x6b: unimplemented_instruction(state); break;
    case 0x6c: unimplemented_instruction(state); break;
    case 0x6d: unimplemented_instruction(state); break;
    case 0x6e: unimplemented_instruction(state); break;
    case 0x6f: { // MOV L,A
      state->l = state->a;
    } break;
    case 0x70: unimplemented_instruction(state); break;
    case 0x71: unimplemented_instruction(state); break;
    case 0x72: unimplemented_instruction(state); break;
    case 0x73: unimplemented_instruction(state); break;
    case 0x74: unimplemented_instruction(state); break;
    case 0x75: unimplemented_instruction(state); break;
    case 0x76: { // HLT
      exit(0); // program halted and so we happily halt ours too
    } break;
    case 0x77: { // MOV M,A
      uint16_t adr = (uint16_t)(state->h << 8) | (state->l);
      state->memory[adr] = state->a;
    } break;
    case 0x78: unimplemented_instruction(state); break;
    case 0x79: unimplemented_instruction(state); break;
    case 0x7a: { // MOV A,D
      state->a = state->d;
    } break;
    case 0x7b: { // MOV A,E
      state->a = state->e;
    } break;
    case 0x7c: { // MOV A,H
      state->a = state->h;
    } break;
    case 0x7d: unimplemented_instruction(state); break;
    case 0x7e: {
      uint16_t hl = (uint16_t)(state->h << 8) | (state->l);
      state->a = state->memory[hl];
    } break;
    case 0x7f: unimplemented_instruction(state); break;
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
    case 0x90: unimplemented_instruction(state); break;
    case 0x91: unimplemented_instruction(state); break;
    case 0x92: unimplemented_instruction(state); break;
    case 0x93: unimplemented_instruction(state); break;
    case 0x94: unimplemented_instruction(state); break;
    case 0x95: unimplemented_instruction(state); break;
    case 0x96: unimplemented_instruction(state); break;
    case 0x97: unimplemented_instruction(state); break;
    case 0x98: unimplemented_instruction(state); break;
    case 0x99: unimplemented_instruction(state); break;
    case 0x9a: unimplemented_instruction(state); break;
    case 0x9b: unimplemented_instruction(state); break;
    case 0x9c: unimplemented_instruction(state); break;
    case 0x9d: unimplemented_instruction(state); break;
    case 0x9e: unimplemented_instruction(state); break;
    case 0x9f: unimplemented_instruction(state); break;
    case 0xa0: unimplemented_instruction(state); break;
    case 0xa1: unimplemented_instruction(state); break;
    case 0xa2: unimplemented_instruction(state); break;
    case 0xa3: unimplemented_instruction(state); break;
    case 0xa4: unimplemented_instruction(state); break;
    case 0xa5: unimplemented_instruction(state); break;
    case 0xa6: unimplemented_instruction(state); break;
    case 0xa7: { // ANA A
      uint16_t answer = (uint16_t)(state->a) & (state->a);
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff, 8);
      state->a = answer & 0xff;
    } break;
    case 0xa8: unimplemented_instruction(state); break;
    case 0xa9: unimplemented_instruction(state); break;
    case 0xaa: unimplemented_instruction(state); break;
    case 0xab: unimplemented_instruction(state); break;
    case 0xac: unimplemented_instruction(state); break;
    case 0xad: unimplemented_instruction(state); break;
    case 0xae: unimplemented_instruction(state); break;
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
    case 0xc4: unimplemented_instruction(state); break;
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
    case 0xc7: unimplemented_instruction(state); break;
    case 0xc8: unimplemented_instruction(state); break;
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
    case 0xcb: unimplemented_instruction(state); break;
    case 0xcc: unimplemented_instruction(state); break;
    case 0xcd: // CALL adr
#ifdef CPUDIAG
      if (5 == ((opcode[2] << 8) | opcode[1])) {
        if (state->c == 9) {
          uint16_t offset = (state->d << 8) | (state->e);
          char *str = &state->memory[offset + 3]; // skip prefix bytes
          while (*str != '$') {
            printf("%c", *str++);
          }
          printf("\n");
        } else if (state->c == 2) {
          printf("print char routine called\n");
        }
      } else if (0 == ((opcode[2] << 8) | opcode[1])) {
        exit(0);
      } else
#endif
    {
      // store return address
      uint16_t ret = state->pc + 2;
      state->memory[state->sp - 1] = (ret >> 8) & 0xff;
      state->memory[state->sp - 2] = (ret & 0xff);
      state->sp -= 2;
      // and jump to next address
      state->pc = (opcode[2] << 8) | (opcode[1]);
      state->pc -= 1;
    } break;
    case 0xce: unimplemented_instruction(state); break;
    case 0xcf: unimplemented_instruction(state); break;
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
    case 0xda: unimplemented_instruction(state); break;
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
    case 0xfa: unimplemented_instruction(state); break;
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
  if (output_in_colour) {
    printf("\tC=\e[35m%d\e[0m, P=\e[35m%d\e[0m, S=\e[35m%d\e[0m, Z=\e[35m%d\e[0m\n", state->cc.cy, state->cc.p, state->cc.s, state->cc.z);
    printf(
      "\tA \e[33m$%02x\e[0m\tB \e[33m$%02x\e[0m\tC \e[33m$%02x\e[0m\t"
      "D \e[33m$%02x\e[0m\tE \e[33m$%02x\e[0m\tH \e[33m$%02x\e[0m\t"
      "L \e[33m$%02x\e[0m\tSP \e[35m%04x\e[0m\n",
      state->a, state->b, state->c, state->d, state->e, state->h, state->l, state->sp
    );
  } else {
    printf("\tC=%d, P=%d, S=%d, Z=%d\n", state->cc.cy, state->cc.p, state->cc.s, state->cc.z);
    printf(
      "\tA $%02x\tB $%02x\tC $%02x\tD $%02x\tE $%02x\tH $%02x\tL $%02x\tSP %04x\n",
      state->a, state->b, state->c, state->d, state->e, state->h, state->l, state->sp
    );
  }

  // output memory visualizer
  if (show_memory) {
    printf("%02x\n", state->memory[0x2000]);

    /* printf("\n"); */
    /* int col_count = 0; */
    /* for (int i = 0x2c10; i < 0x3440; i++) { */
    /*   printf("%02x ", state->memory[i]); */
    /*   if (col_count > 40) { */
    /*     col_count = 0; */
    /*     printf("\n"); */
    /*   } */
    /*   col_count++; */
    /* } */
  }
}
