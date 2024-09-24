#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void unimplemented_instruction(State *state);
uint8_t parity(uint8_t);
void emulate(State *state);


int main(int argc, char *argv[]) {
  return 0;
}


// instruction functions
void unimplemented_instruction(State *state) {
  printf("Error: instruction not implemented!\n");
  exit(1);
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
    case 0x0f: unimplemented_instruction(state); break;
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
    case 0x1f: unimplemented_instruction(state); break;
    /* .. */
    case 0x2f: { // CMA (not)
      state->a = ~state->a;
      // flags are not affected
    } break;
    /* .. */
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
      state->cc.p = parity(answer & 0xff);
      // now store in A
      state->a = answer & 0xff;
    } break;
    case 0x81: { // ADD C
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->c;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x82: { // ADD D
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->d;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x83: { // ADD E
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->e;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x84: { // ADD H
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->h;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x85: { // ADD L
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->l;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x86: { // ADD M
      // copy from memory into accumulator
      uint16_t offset = (state->h << 8) | (state->l);
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->memory[offset];
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x87: { // ADD A
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->a;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x88: { // ADC B
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->b + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x89: { // ADC C
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->c + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x8a: { // ADC D
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->d + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x8b: { // ADC E
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->e + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x8c: { // ADC H
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->h + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x8d: { // ADC L
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->l + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x8e: { // ADC M
      // copy from memory into accumulator
      uint16_t offset = (state->h << 8) | (state->l);
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->memory[offset] + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    case 0x8f: { // ADC A
      uint16_t answer = (uint16_t)state->a + (uint16_t)state->a + (uint16_t)state->cc.cy;
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
    } break;
    /* .. */
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
    /* .. */
    case 0xc6: { // ADI D8
      // we add immediate, so opcode[1] fetches the next byte
      uint16_t answer = (uint16_t)state->a + (uint16_t)opcode[1];
      state->cc.z = ((answer & 0xff) == 0);
      state->cc.s = ((answer & 0x80) != 0);
      state->cc.cy = (answer > 0xff);
      state->cc.p = parity(answer & 0xff);
      state->a = answer & 0xff;
      state->pc += 1;
    } break;
    /* .. */
    case 0xc9: { // RET
      // jump to previously store return address
      state->pc = state->memory[state->sp] | (state->memory[state->sp + 1] << 8);
      state->sp += 2;
    } break;
    /* .. */
    case 0xcd: { // CALL adr
      // store return address
      uint16_t ret = state->pc + 2;
      state->memory[state->sp - 1] = (ret >> 8) & 0xff;
      state->memory[state->sp - 2] = (ret & 0xff);
      state->sp = state->sp - 2;
      // and jump to next address
      state->pc = (opcode[2] << 8) | (opcode[1]);
    } break;
    /* .. */
    case 0xe6: { // ANI byte
      uint8_t x = state->a & opcode[1];
      state->cc.z = (x == 0);
      state->cc.s = (0x80 == (x & 0x80));
      state->cc.p = parity(x);
      state->cc.cy = 0; // ANI clears CY
      state->a = x;
      state->pc++; // we used next byte, so increment
    } break;
  }

  state->pc += 1;
}
