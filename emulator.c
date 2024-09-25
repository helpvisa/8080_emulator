#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void unimplemented_instruction(State *state);
uint8_t parity(int num, int bits);
void emulate(State *state);


int main(int argc, char *argv[]) {
  return 0;
}


// instruction functions
void unimplemented_instruction(State *state) {
  printf("Error: instruction not implemented!\n");
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
    case 0x03: unimplemented_instruction(state); break;
    case 0x04: unimplemented_instruction(state); break;
    case 0x05: unimplemented_instruction(state); break;
    case 0x06: unimplemented_instruction(state); break;
    case 0x07: unimplemented_instruction(state); break;
    case 0x08: unimplemented_instruction(state); break;
    case 0x09: unimplemented_instruction(state); break;
    case 0x0a: unimplemented_instruction(state); break;
    case 0x0b: unimplemented_instruction(state); break;
    case 0x0c: unimplemented_instruction(state); break;
    case 0x0d: unimplemented_instruction(state); break;
    case 0x0e: unimplemented_instruction(state); break;
    case 0x0f: { // RRC
      uint8_t x = state->a;
      state->a = ((x & 1) << 7) | (x >> 1);
      state->cc.cy = (1 == (x & 1));
    } break;
    case 0x10: unimplemented_instruction(state); break;
    case 0x11: unimplemented_instruction(state); break;
    case 0x12: unimplemented_instruction(state); break;
    case 0x13: unimplemented_instruction(state); break;
    case 0x14: unimplemented_instruction(state); break;
    case 0x15: unimplemented_instruction(state); break;
    case 0x16: unimplemented_instruction(state); break;
    case 0x17: unimplemented_instruction(state); break;
    case 0x18: unimplemented_instruction(state); break;
    case 0x19: unimplemented_instruction(state); break;
    case 0x1a: unimplemented_instruction(state); break;
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
    case 0x21: unimplemented_instruction(state); break;
    case 0x22: unimplemented_instruction(state); break;
    case 0x23: unimplemented_instruction(state); break;
    case 0x24: unimplemented_instruction(state); break;
    case 0x25: unimplemented_instruction(state); break;
    case 0x26: unimplemented_instruction(state); break;
    case 0x27: unimplemented_instruction(state); break;
    case 0x28: unimplemented_instruction(state); break;
    case 0x29: unimplemented_instruction(state); break;
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
    case 0x31: unimplemented_instruction(state); break;
    case 0x32: unimplemented_instruction(state); break;
    case 0x33: unimplemented_instruction(state); break;
    case 0x34: unimplemented_instruction(state); break;
    case 0x35: unimplemented_instruction(state); break;
    case 0x36: unimplemented_instruction(state); break;
    case 0x37: unimplemented_instruction(state); break;
    case 0x38: unimplemented_instruction(state); break;
    case 0x39: unimplemented_instruction(state); break;
    case 0x3a: unimplemented_instruction(state); break;
    case 0x3b: unimplemented_instruction(state); break;
    case 0x3c: unimplemented_instruction(state); break;
    case 0x3d: unimplemented_instruction(state); break;
    case 0x3e: unimplemented_instruction(state); break;
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
    case 0x56: unimplemented_instruction(state); break;
    case 0x57: unimplemented_instruction(state); break;
    case 0x58: unimplemented_instruction(state); break;
    case 0x59: unimplemented_instruction(state); break;
    case 0x5a: unimplemented_instruction(state); break;
    case 0x5b: unimplemented_instruction(state); break;
    case 0x5c: unimplemented_instruction(state); break;
    case 0x5d: unimplemented_instruction(state); break;
    case 0x5e: unimplemented_instruction(state); break;
    case 0x5f: unimplemented_instruction(state); break;
    case 0x60: unimplemented_instruction(state); break;
    case 0x61: unimplemented_instruction(state); break;
    case 0x62: unimplemented_instruction(state); break;
    case 0x63: unimplemented_instruction(state); break;
    case 0x64: unimplemented_instruction(state); break;
    case 0x65: unimplemented_instruction(state); break;
    case 0x66: unimplemented_instruction(state); break;
    case 0x67: unimplemented_instruction(state); break;
    case 0x68: unimplemented_instruction(state); break;
    case 0x69: unimplemented_instruction(state); break;
    case 0x6a: unimplemented_instruction(state); break;
    case 0x6b: unimplemented_instruction(state); break;
    case 0x6c: unimplemented_instruction(state); break;
    case 0x6d: unimplemented_instruction(state); break;
    case 0x6e: unimplemented_instruction(state); break;
    case 0x6f: unimplemented_instruction(state); break;
    case 0x70: unimplemented_instruction(state); break;
    case 0x71: unimplemented_instruction(state); break;
    case 0x72: unimplemented_instruction(state); break;
    case 0x73: unimplemented_instruction(state); break;
    case 0x74: unimplemented_instruction(state); break;
    case 0x75: unimplemented_instruction(state); break;
    case 0x76: unimplemented_instruction(state); break;
    case 0x77: unimplemented_instruction(state); break;
    case 0x78: unimplemented_instruction(state); break;
    case 0x79: unimplemented_instruction(state); break;
    case 0x7a: unimplemented_instruction(state); break;
    case 0x7b: unimplemented_instruction(state); break;
    case 0x7c: unimplemented_instruction(state); break;
    case 0x7d: unimplemented_instruction(state); break;
    case 0x7e: unimplemented_instruction(state); break;
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
    case 0xa7: unimplemented_instruction(state); break;
    case 0xa8: unimplemented_instruction(state); break;
    case 0xa9: unimplemented_instruction(state); break;
    case 0xaa: unimplemented_instruction(state); break;
    case 0xab: unimplemented_instruction(state); break;
    case 0xac: unimplemented_instruction(state); break;
    case 0xad: unimplemented_instruction(state); break;
    case 0xae: unimplemented_instruction(state); break;
    case 0xaf: unimplemented_instruction(state); break;
    case 0xb0: unimplemented_instruction(state); break;
    case 0xb1: unimplemented_instruction(state); break;
    case 0xb2: unimplemented_instruction(state); break;
    case 0xb3: unimplemented_instruction(state); break;
    case 0xb4: unimplemented_instruction(state); break;
    case 0xb5: unimplemented_instruction(state); break;
    case 0xb6: unimplemented_instruction(state); break;
    case 0xb7: unimplemented_instruction(state); break;
    case 0xb8: unimplemented_instruction(state); break;
    case 0xb9: unimplemented_instruction(state); break;
    case 0xba: unimplemented_instruction(state); break;
    case 0xbb: unimplemented_instruction(state); break;
    case 0xbc: unimplemented_instruction(state); break;
    case 0xbd: unimplemented_instruction(state); break;
    case 0xbe: unimplemented_instruction(state); break;
    case 0xbf: unimplemented_instruction(state); break;
    case 0xc0: unimplemented_instruction(state); break;
    case 0xc1: unimplemented_instruction(state); break;
    case 0xc2: { // JNZ adr
      if (0 == state->cc.z) {
        state->pc = (opcode[2] << 8) | (opcode[1]);
      } else {
        state->pc += 2;
      }
    } break;
    case 0xc3: { // JMP adr
      state->pc = (opcode[2] << 8) | (opcode[1]);
    } break;
    case 0xc4: unimplemented_instruction(state); break;
    case 0xc5: unimplemented_instruction(state); break;
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
      state->sp += 2;
    } break;
    case 0xca: unimplemented_instruction(state); break;
    case 0xcb: unimplemented_instruction(state); break;
    case 0xcc: unimplemented_instruction(state); break;
    case 0xcd: { // CALL adr
      // store return address
      uint16_t ret = state->pc + 2;
      state->memory[state->sp - 1] = (ret >> 8) & 0xff;
      state->memory[state->sp - 2] = (ret & 0xff);
      state->sp = state->sp - 2;
      // and jump to next address
      state->pc = (opcode[2] << 8) | (opcode[1]);
    } break;
    case 0xce: unimplemented_instruction(state); break;
    case 0xcf: unimplemented_instruction(state); break;
    case 0xd0: unimplemented_instruction(state); break;
    case 0xd1: unimplemented_instruction(state); break;
    case 0xd2: unimplemented_instruction(state); break;
    case 0xd3: unimplemented_instruction(state); break;
    case 0xd4: unimplemented_instruction(state); break;
    case 0xd5: unimplemented_instruction(state); break;
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
    case 0xe1: unimplemented_instruction(state); break;
    case 0xe2: unimplemented_instruction(state); break;
    case 0xe3: unimplemented_instruction(state); break;
    case 0xe4: unimplemented_instruction(state); break;
    case 0xe5: unimplemented_instruction(state); break;
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
    case 0xeb: unimplemented_instruction(state); break;
    case 0xec: unimplemented_instruction(state); break;
    case 0xed: unimplemented_instruction(state); break;
    case 0xee: unimplemented_instruction(state); break;
    case 0xef: unimplemented_instruction(state); break;
    case 0xf0: unimplemented_instruction(state); break;
    case 0xf1: unimplemented_instruction(state); break;
    case 0xf2: unimplemented_instruction(state); break;
    case 0xf3: unimplemented_instruction(state); break;
    case 0xf4: unimplemented_instruction(state); break;
    case 0xf5: unimplemented_instruction(state); break;
    case 0xf6: unimplemented_instruction(state); break;
    case 0xf7: unimplemented_instruction(state); break;
    case 0xf8: unimplemented_instruction(state); break;
    case 0xf9: unimplemented_instruction(state); break;
    case 0xfa: unimplemented_instruction(state); break;
    case 0xfb: unimplemented_instruction(state); break;
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

  state->pc += 1;
}
