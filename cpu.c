#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "disassembler.h"
#include "cpu.h"

#define DEBUG 0

static int parity(int x, int size) {
    // this parity might not work
    // int PositiveBits = 0;
    // for (int c = 7; c >= 0; c--)
    // {
    //   int k = value >> c;

    //   if (k & 1)
    //   {
    //     PositiveBits += 1;
    //   }
    // }
    // if (PositiveBits % 2)
    // {
    //     state->cc.p = 1;
    // }
    // else
    // {
    //     state->cc.p = 0;
    // }
    uint8_t nb_one_bits = 0;
  for (int i = 0; i < 8; i++) {
    nb_one_bits += ((x >> i) & 1);
  }

  return (nb_one_bits & 1) == 0;
    // int i;
	// int p = 0;
	// x = (x & ((1<<size)-1));
	// for (i=0; i<size; i++)
	// {
	// 	if (x & 0x1) p++;
	// 	x = x >> 1;
	// }
	// return (0 == (p & 0x1));
}

void checkZSP(uint8_t value, State8080 *state) {
    // check zero
    state->cc.z = (value == 0);
    state->cc.s = (0x80 == (value & 0x80));
    state->cc.p = parity(value, 8);    
    // if ((value && 0xff) == 0) {
    //     state->cc.z = 1;
    // }
    // else {
    //     state->cc.z = 0;
    // }
    // // check sign
    // if (value & 0x80) {
    //     state->cc.s = 1;
    // }
    // else {
    //     state->cc.s = 0;
    // }
    

    // state->cc.p = parity(value & 0xff, 8);
}

void checkA(uint16_t value, State8080 *state) {
    // check zero
    // if ((value && 0xff) == 0) {
    //     state->cc.z = 1;
    // }
    // else {
    //     state->cc.z = 0;
    // }
    // // check sign
    // if (value & 0x80) {
    //     state->cc.s = 1;
    // }
    // else {
    //     state->cc.s = 0;
    // }
    state->cc.z = ((value&0xff) == 0);
	state->cc.s = (0x80 == (value & 0x80));
    
    state->cc.p = parity(value & 0xff, 8);
    // Check carry
    state->cc.cy = (value >= 0xff);
}

void checkOpA(State8080 *state) {
    //Logic
    state->cc.cy = 0;
    // state->cc.ac = ((state->a & 0x0f)  ^ (value & 0x0f) > 0xf);
    // state->cc.ac = 0;
    // state->cc.cy = state->cc.ac = 0;
    state->cc.z = (state->a == 0);
    state->cc.s = (0x80 == (state->a & 0x80));
    state->cc.p = parity(state->a, 8);
}

void CheckFlags(uint16_t result, State8080* state)
{
    if (result & 0xff)
    {
        state->cc.z = 0;
    }
    else
    {
        state->cc.z = 1;
    }

    if (result & 0x80)
    {
        state->cc.s = 1;
    }
    else
    {
        state->cc.s = 0;
    }

    if (result > 0xff)
    {
        state->cc.cy = 1;
    }
    else
    {
        state->cc.cy = 0;
    }


    int PositiveBits = 0;
    for (int c = 7; c >= 0; c--)
    {
      int k = result >> c;

      if (k & 1)
      {
        PositiveBits += 1;
      }
    }
    if (PositiveBits % 2)
    {
        state->cc.p = 1;
    }
    else
    {
        state->cc.p = 0;
    }
}


int call(State8080 *state, unsigned char *opcode, int *cycles) {
//CALL
#if DEBUG
                    // unsigned char *tmp = &state->memory[state->pc + 1];
                    // unsigned char *test = &state->memory[state->pc + 1];
                    // if (5 == ((opcode[2] << 8) | opcode[1]))
                    // if ()
                    // if (5 == *test)
                    if (state->pc == 5)
                    {    
                        if (state->c == 9)   
                        {    
                            uint16_t offset = (state->d<<8) | (state->e);    
                            char *str = &state->memory[offset+3];  //skip the prefix bytes    
                            while (*str != '$')    
                                printf("%c", *str++);    
                            printf("\n");
                            return 0;  
                        }    
                        else if (state->c == 2)    
                        {    
                            //saw this in the inspected code, never saw it called    
                            // printf ("print char routine called\n");    
                            printf("%c", state->e);
                            return 0;
                        }    
                    }
                    // else if (*test == 0)
                    else if (0 ==  ((opcode[2] << 8) | opcode[1]))    
                    {    
                        exit(0);    
                    }
                    else
                    #endif
                    {

                    }
                        uint16_t    ret = state->pc + 2;    
                        state->sp = state->sp - 2;
                        state->memory[state->sp + 1] = (ret >> 8) & 0xff;       
                        state->memory[state->sp] = (ret & 0xff);
                        state->pc = (opcode[2] << 8) | opcode[1];
                        return 1;
                    
cycles += 17;   
}

unsigned char cycles8080[] = {
	4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4, //0x00..0x0f
	4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4, //0x10..0x1f
	4, 10, 16, 5, 5, 5, 7, 4, 4, 10, 16, 5, 5, 5, 7, 4, //etc
	4, 10, 13, 5, 10, 10, 10, 4, 4, 10, 13, 5, 5, 5, 7, 4,
	
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5, //0x40..0x4f
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
	7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 7, 5,
	
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, //0x80..8x4f
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	
	11, 10, 10, 10, 17, 11, 7, 11, 11, 10, 10, 10, 10, 17, 7, 11, //0xc0..0xcf
	11, 10, 10, 10, 17, 11, 7, 11, 11, 10, 10, 10, 10, 17, 7, 11, 
	11, 10, 10, 18, 17, 11, 7, 11, 11, 5, 10, 5, 17, 17, 7, 11, 
	11, 10, 10, 4, 17, 11, 7, 11, 11, 5, 10, 4, 17, 17, 7, 11, 
};

static const uint8_t OPCODES_CYCLES[256] = {
//  0  1   2   3   4   5   6   7   8  9   A   B   C   D   E  F
    4, 10, 7,  5,  5,  5,  7,  4,  4, 10, 7,  5,  5,  5,  7, 4,  // 0
    4, 10, 7,  5,  5,  5,  7,  4,  4, 10, 7,  5,  5,  5,  7, 4,  // 1
    4, 10, 16, 5,  5,  5,  7,  4,  4, 10, 16, 5,  5,  5,  7, 4,  // 2
    4, 10, 13, 5,  10, 10, 10, 4,  4, 10, 13, 5,  5,  5,  7, 4,  // 3
    5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7, 5,  // 4
    5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7, 5,  // 5
    5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7, 5,  // 6
    7, 7,  7,  7,  7,  7,  7,  7,  5, 5,  5,  5,  5,  5,  7, 5,  // 7
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  // 8
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  // 9
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  // A
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7, 4,  // B
    5, 10, 10, 10, 11, 11, 7,  11, 5, 10, 10, 10, 11, 17, 7, 11, // C
    5, 10, 10, 10, 11, 11, 7,  11, 5, 10, 10, 10, 11, 17, 7, 11, // D
    5, 10, 10, 18, 11, 11, 7,  11, 5, 5,  10, 4,  11, 17, 7, 11, // E
    5, 10, 10, 4,  11, 11, 7,  11, 5, 5,  10, 4,  11, 17, 7, 11  // F
};

void UnimplementedInstruction(State8080* state)
{
    //pc will have advanced one, so undo that
    printf("Error: Unimplemented instruction\n");
    printf("%d", state->memory[state->pc]);
    exit(1);
}

uint16_t machineIN(State8080 state, uint8_t port) {
    uint16_t a;
    switch(port) {
        printf("%d\n", port);
        // case 0:
            // return 1;
        case 1:
            return state.port1;
        // case 2:
            // return 0x0;
        case 3: {
            uint16_t v = (state.shift1 << 8) | state.shift0;
            a = ((v >> (8 - state.shift_offset)) & 0xff);
            break;
        }
    }
    return a;
}


// void machineOUT(uint16_t port, State8080 *state) {
//     #if DEBUG
//         if (port == 0) {
//             printf("DONE\n");
//         }
//         else if (port == 1) {
//             uint8_t operation = state->c;
            
//             if (operation == 2) {
//                 printf("%c", state->e);
//             }
//             else if (operation == 0) {
//                 uint16_t addr = (state->d << 8) | state->e;
//                 do {
//                     printf("%c", state->memory[addr++]);
//                 } while (state->memory[addr] != '$');
//             }
//         }
//     #endif
//     switch(port) {
//         case 1:
//             {
//                 uint8_t test = state->c;
//                 if (test == 2) {
//                     printf("%c", &state->e);
//                     // printf(state->e);
//                 }
//                 else if (test == 9) {
//                     uint16_t addr = (state->d << 8) | state->e;
//                     do {
//                         printf("%c", state->memory[addr++]);
//                     }
//                     while(state->memory[addr] != '$');
//                 }
//                 break;
//             }
//         case 2:
//             state->shift_offset = state->a & 0x7;
//             break;
//         case 3:
//             state->outport3 = state->a;
//             break;
//         case 4:
//             state->shift0 = state->shift1;
//             state->shift1 = state->a;
//             break;
//         case 5:
//             state->outport5 = state->a;
//             break;
//     }
// }


int Emulate8080Op(State8080* state)
{
    int cycles = 0;

    unsigned char *opcode = &state->memory[state->pc];
                    state->pc++;
            // Disassemble8080Op(opcode, state->pc);
        // while (cycles < 17066)
        // { 
            // printf("DEBUG: %X\n", *opcode);
            // Disassemble8080Op(state->memory, state->pc);
            switch(*opcode)
            {
                case 0x00:
                {
                    cycles += 4;
                    break;
                }
                case 0x01:
                {
                    //LXI
                    state->c = opcode[1];
                    state->b = opcode[2];
                    state->pc += 2;
                    cycles += 10;
                    break;
                }
                case 0x02:
                {
                    //STAX
                    uint16_t memory_address = (state->b<<8) | state->c;
                    state->memory[memory_address] = state->a;
                    cycles += 7;
                    break;
                }
                case 0x03:
                {
                    //INX
                    // ***
                    uint16_t pair = (state->b<<8) | state->c;
                    pair++;
                    state->b = pair >> 8;
                    state->c = pair & 0xff;
                    cycles += 5;
                    break;
                }
                case 0x04:
                {
                    //INR
                    checkZSP(state->b + 1, state);
                    state->b++;
                    state->cc.ac = ((state->b & 0x0f)  + (1 & 0x0f) > 0xf);
                    cycles += 5;
                    break;
                }
                case 0x05:
                {
                    //DCR
                    state->b--;
                    state->cc.ac = !((state->b & 0xf) == 0xf);
                    checkZSP(state->b, state);
                    cycles += 5;
                    break;
                }
                case 0x06:
                {
                    //MVI
                    // ***
                    state->b = opcode[1];
                    state->pc += 1;
                    cycles += 7;
                    break;
                }
                case 0x07:
                {
                    //RLC
                    // setting the carry flag here might not work
                    uint8_t tmp = state->a;
                    state->a = ((tmp & 0x80) >> 7) | (tmp << 1);
                    state->cc.cy = (0x80 == (tmp & 0x80)); 
                    // state->cc.cy = ((state->a & 0xffff0000) != 0);
                    cycles += 4;
                    break;
                }
                case 0x08:
                {
                    cycles += 4;
                    break;
                }
                case 0x09:
                {
                    //DAD
                    // ***
                    uint16_t BC = (state->b<<8) | state->c;
                    uint16_t HL = (state->h<<8) | state->l;
                    HL += BC;
                    // Note: might be broken MAY
                    state->h = HL>>8;
                    state->l = HL & 0xff;
                    // checkZSP(HL, state);
                    // state->cc.cy = ((HL & 0xffff0000) != 0); // NOTE CHECK this
                    state->cc.cy = (HL > 0xff);
                    cycles += 10;
                    break;
                }
                case 0x0a:
                {
                    //LDAX B
                    uint16_t memory_address = (state->b << 8) | state->c;
                    state->a = state->memory[memory_address];
                    cycles += 7;
                    break;
                }
                case 0x0b:
                {
                    //DCX
                    // ***
                    uint16_t bc = (state->b << 8) | state->c;
                    bc--;
                    state->b = bc >> 8;
                    state->c = bc & 0xff; // NOTE might have to make this save only the last 8 bits of the uint16_t
                    cycles += 5;
                    break;
                }
                case 0x0c:
                {
                    // INR C
                    state->cc.ac = ((state->c & 0x0f)  + (1 & 0x0f) > 0xf);
                    state->c++;
                    checkZSP(state->c, state); 
                    cycles += 5;
                    break;
                }
                case 0x0d:
                {
                    //DCR
                    state->c--;
                    state->cc.ac = !((state->c & 0xf) == 0xf);
                    checkZSP(state->c, state);
                    cycles += 5;
                    break;
                }
                case 0x0e:
                {
                    //MVI
                    // ***
                    state->c = opcode[1];
                    state->pc += 1;
                    cycles += 7;
                    break;
                }
                case 0x0f:
                {
                    //RRC
                    // state->cc.cy = (1 == (state->a & 1));
                    uint8_t x = state->a;
                    // state->cc.cy = state->a >> 6;
                    state->a = ((x & 1) << 7) | (x >> 1);
                    state->cc.cy = (1 == (x&1));
                    // CheckFlags(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x10:
                {
                    cycles += 4;
                    break;
                }
                case 0x11:
                {
                    //LXI
                    state->e = opcode[1];
                    state->d = opcode[2];
                    state->pc += 2;
                    cycles += 10;
                    break;
                }
                case 0x12:
                {
                    //STAX D
                    uint16_t memory_address = (state->d << 8) | state->e;
                    state->memory[memory_address] = state->a;
                    cycles += 7;
                    break;
    
                }
                case 0x13:
                {
                    //INX
                    uint16_t pair = (state->d << 8) | state->e;
                    pair++;
                    state->d = pair >> 8;
                    state->e = pair & 0xff;
                    cycles += 5;
                    break;
                }
                case 0x14:
                {
                    //INR D
                    state->cc.ac = ((state->d & 0x0f) + (1 & 0x0f) > 0xf);
                    state->d++;
                    checkZSP(state->d, state); // NOTE Might have to pass state by reference
                    cycles += 5;
                    break;
                }
                case 0x15:
                {
                    //DCR D
                    state->d--;
                    checkZSP(state->d, state);
                    state->cc.ac = !((state->d & 0xf) == 0xf);
                    cycles +=5;
                    break;
                }
                case 0x16:
                {
                    //MVI D,D8
                    // ***
                    // state->d = state->memory[state->pc+1] & 0xff; //NOTE Chec this
                    state->d = opcode[1];
                    state->pc++;
                    cycles +=7;
                    break;
                }
                case 0x17:
                {
                    //RAL
                    // state->cc.cy = state->a >> 6;
                    uint8_t val = state->a;
                    state->a = ((state->cc.cy) | (val << 1));
                    // CheckFlags(state->a, state);
                    state->cc.cy = (0x80 == (val & 0x80));
                    cycles += 4;
                    break;
                }
                case 0x18:
                {
                    cycles += 4;
                    break;
                }
                case 0x19:
                {
                    //DAD
                    uint16_t DE = (state->d << 8) | state->e;
                    uint16_t HL = (state->h << 8) | state->l;
                    HL += DE;
                    state->h = HL >> 8;
                    state->l = HL & 0xff;
                    // CheckFlags(HL, state);
                    state->cc.cy = ((HL & 0xffff0000) != 0);
                    cycles += 10;
                    break;
                }
                case 0x1a:
                {
                    //LDAX
                    uint16_t memory_address = (state->d << 8) | state->e;
                    state->a = state->memory[memory_address];
                    cycles += 7;
                    break;
                }
                case 0x1b:
                {
                    //DCX
                    uint16_t de = (state->d << 8) | state->e;
                    de--;
                    state->d = de >> 8;
                    state->e = de & 0xff; // NOTE might have to make this save only the last 8 bits of the uint16_t
                    cycles += 5;
                    break;
                }
                case 0x1c:
                {
                    state->cc.ac = ((state->e & 0x0f)  + (1 & 0x0f) > 0xf);
                    state->e++;
                    checkZSP(state->e, state);
                    cycles += 5;
                    break;
                }
                case 0x1d:
                {
                    state->e--;
                    checkZSP(state->e, state);
                    state->cc.ac = !((state->e & 0xf) == 0xf);
                    cycles += 5;
                    break;
                }
                case 0x1e:
                {
                    //MVI E,D8
                    state->e = opcode[1];
                    // state->e = state->memory[state->pc+1] & 0xff; //NOTE Chec this
                    state->pc++;
                    cycles += 7;
                    break;
                }
                case 0x1f:
                {
                    //RAR
                    uint8_t val = state->a;
                    state->a = (state->cc.cy << 7) | (val >> 1);
                    state->cc.cy = (1 == (val & 1));
                    cycles += 4;
                    break;
                }
                case 0x20:
                {
                    cycles += 4;
                    break;
                }
                case 0x21:
                {
                    //LXI
                    state->l = opcode[1];
                    state->h = opcode[2];
                    state->pc += 2;
                    cycles += 10;
                    break;
                }
                case 0x22:
                {
                    // uint16_t memory_address = (state->h << 8) | state->l;
                    uint16_t memory_address = (opcode[2] << 8) | opcode[1];
                    state->memory[memory_address] = state->l;
                    state->memory[memory_address+1] = state->h; // NOTE Check that the + 1 works as intended
                    state->pc += 2;
                    cycles += 16;
                    break;
                }
                case 0x23:
                {
                    //INX
                    uint16_t pair = (state->h << 8) | state->l;
                    pair++;
                    state->h = pair >> 8;
                    state->l = pair & 0xff;
                    cycles += 5;
                    break;
                }
                case 0x24:
                {
                    state->cc.ac = ((state->h & 0x0f)  + (1 & 0x0f) > 0xf);
                    state->h++;
                    checkZSP(state->h, state);
                    cycles = 5;
                    break;
                }
                case 0x25:
                {
                    state->h--;
                    checkZSP(state->h, state);
                    state->cc.ac = !((state->h & 0xf) == 0xf);
                    cycles = 5;
                    break;
                }
                case 0x26:
                {
                    //MVI
                    state->h = opcode[1];
                    state->pc += 1;
                    cycles += 7;
                    break;
                }
                case 0x27:
                {
                    //DAA
                    uint16_t result = state->a;
                    int cy = state->cc.cy;
printf("%d\n", result);
                    if ((state->a & 0xf) > 9 || state->cc.ac) 
                    {
                        result += 6;
                    }
                    if ((state->a >> 4) > 9 || state->cc.cy || ((state->a >> 4) >= 9 || (state->a & 0xf) > 9))
                    {
                        result += 0x60;
                        cy = 1;
                        // printf("here");
                    }
                    checkA(result, state);
                    // printf("%d\n", result);
                    state->cc.cy = cy;
                    // state->cc.ac = ((result)) > 0xf;
                    state->cc.ac = (unsigned)((((~result) & 0xf) + 1) > 0xf);
                    state->a = result;
                    cycles += 4;
                    break;
                }
                case 0x28:
                {
                    cycles += 4;
                    break;
                }
                case 0x29:
                {
                    //DAD
                    uint16_t HL = (state->h << 8) | state->l;
                    HL *= 2;
                    state->h = HL >> 8;
                    state->l = HL & 0xff;
                    // CheckFlags(HL, state);
                    state->cc.cy = ((HL & 0xffff0000) != 0);
                    cycles += 10;
                    break;
                }
                case 0x2a:
                {
                    // uint16_t memory_address = (state->h << 8) | state->l;
                    uint16_t memory_address = (opcode[2] << 8) | opcode[1];
                    state->l = state->memory[memory_address];
                    state->h = state->memory[memory_address+1]; // NOTE Check that the + 1 works as intended
                    state->pc += 2;
                    cycles += 16;
                    break;
                }
                case 0x2b:
                {
                    //DCX H
                    uint16_t hl = (state->h << 8) | state->l;
                    hl--;
                    state->h = hl >> 8;
                    state->l = hl & 0xff; // NOTE might have to make this save only the last 8 bits of the uint16_t
                    cycles += 5;
                    break;
                }
                case 0x2c:
                {
                    //INR L
                    state->cc.ac = ((state->l & 0x0f)  + (1 & 0x0f) > 0xf);
                    state->l++;
                    checkZSP(state->l, state);
                    cycles += 5;
                    break;
                }
                case 0x2d:
                {
                    //DCR L
                    state->l--;
                    checkZSP(state->l, state);
                    state->cc.ac = !((state->l & 0xf) == 0xf);
                    cycles += 5;
                    break;
                }
                case 0x2e:
                {
                    //MVI L, D8
                    state->l = opcode[1];
                    // state->l = state->memory[state->pc+1] & 0xff; //NOTE Chec this
                    state->pc++;
                    cycles += 7;
                    break;
                }
                case 0x2f:
                {
                    state->a = ~state->a; //NOTE check this
                    cycles += 4;
                    break;
                }
                case 0x30:
                {
                    cycles += 4;
                    break;
                }
                case 0x31:
                {
                    //LXI
                    // *** THIS DEFINATLY DOES NOT WORK
                    // CHECK OTHER OPCODES LIKE LXI
                    // Set stack pointer equal to opcode[2] << 8 | opcode[1]
                    // Just fixed May 
                    state->sp = (opcode[2] << 8) | opcode[1];
                    // uint8_t high = state->sp >> 8;
                    // uint8_t low = state->sp & 0xff;
                    // low = opcode[1];
                    // high = opcode[2];
                    // state->sp = (high << 8) | low;
                    state->pc += 2;
                    cycles += 10;
                    break;
                }
                case 0x32:
                {
                    //STA
                    uint16_t memory_address = (opcode[2] << 8) | opcode[1];
                    state->memory[memory_address] = state->a;
                    state->pc += 2;
                    cycles += 13;
                    break;
                }
                case 0x33:
                {
                    state->sp++;
                    cycles += 5;
                    break;
                }
                case 0x34:
                {
                    //INR M
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->cc.ac = ((state->memory[memory_address] & 0x0f) + (1 & 0x0f) > 0xf); // Double check this when testing.
                    state->memory[memory_address] = (state->memory[memory_address] + 1);
                    checkZSP(state->memory[memory_address], state);
                    cycles += 10;
                    break;
                }
                case 0x35:
                {
                    // DCR M
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->memory[memory_address] = (state->memory[memory_address] - 1);
                    checkZSP(state->memory[memory_address], state);
                    state->cc.ac = !((state->memory[memory_address] & 0xf) == 0xf);
                    cycles += 10;
                    break;
                }
                case 0x36:
                {
                    //MVI
                    uint16_t HL = (state->h << 8) | state->l;
                    state->memory[HL] = opcode[1];
                    state->pc += 1;
                    cycles += 10;
                    break;
                }
                case 0x37:
                {
                    state->cc.cy = 1;
                    // CheckFlags(state->cc.cy, state);
                    cycles += 4; // NOTE CHANGED 
                    break;
                }
                case 0x38:
                {
                    cycles += 4;
                    break;
                }
                case 0x39:
                {
                    uint16_t HL = (state->h << 8) | state->l;
                    HL = HL + state->sp;
                    state->h = (HL & 0xff00) >> 8;
                    state->l = HL & 0xFF;
                    state->cc.cy = ((HL & 0xffff0000) > 0);
                    cycles += 10;
                    break;
                }
                case 0x3a:
                {
                    //LDA
                    uint16_t memory_address = (opcode[2] << 8) | opcode[1];
                    state->a = state->memory[memory_address];
                    state->pc += 2;
                    cycles += 13;
                    break;
                }
                case 0x3b:
                {
                    state->sp--;
                    cycles += 5;
                    break;
                }
                case 0x3c:
                {
                    state->cc.ac = ((state->a & 0x0f)  + (1 & 0x0f) > 0xf);
                    state->a++;
                    checkZSP(state->a, state);
                    cycles += 5;
                    break;
                }
                case 0x3d:
                {
                    state->a--;
                    checkZSP(state->a, state);
                    state->cc.ac = !((state->a & 0xf) == 0xf);
                    cycles += 5;
                    break;
                }
                case 0x3e:
                {
                    //MVI
                    state->a = opcode[1];
                    state->pc += 1;
                    cycles += 7;
                    break;
                }
                case 0x3f:
                {
                    //CMC
                    state->cc.cy = 0;//!state->cc.cy; // might have to change to state->cc.cy = 0
                    cycles += 4;
                    break;
                }
                case 0x40:
                {
                    state->b = state->b;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x41:
                {
                    state->b = state->c;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x42:
                {
                    state->b = state->d;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x43:
                {
                    state->b = state->e;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x44:
                {
                    state->b = state->h;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x45:
                {
                    state->b = state->l;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x46:
                {
                    // uint16_t memory_address = (opcode[2] << 8) | opcode[1];
                    uint16_t memory_address = (state->h << 8) | state->l;
                    // printf("%d\n", memory_address);
                    // printf("%d\n", state->memory[memory_address]);
                    state->b = state->memory[memory_address];
                    cycles += 7;
                    //state->pc++;
                    break;
                }
                case 0x47:
                {
                    state->b = state->a;
                    cycles += 5;
                    break;
                }
                case 0x48:
                {
                    state->c = state->b;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x49:
                {
                    state->c = state->c;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x4a:
                {
                    state->c = state->d;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x4b:
                {
                    state->c = state->e;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x4c:
                {
                    state->c = state->h;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x4d:
                {
                    state->c = state->l;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x4e:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->c = state->memory[memory_address];
                    cycles += 7;
                    //state->pc++;
                    break;
                }
                case 0x4f:
                {
                    state->c = state->a;
                    cycles += 5;
                    break;
                }
                case 0x50:
                {
                    state->d = state->b;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x51:
                {
                    state->d = state->c;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x52:
                {
                    state->d = state->d;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x53:
                {
                    state->d = state->e;
                    cycles += 5;
                    //state->pc++;
                    break;
                }
                case 0x54:
                {
                    state->d = state->h;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x55:
                {
                    state->d = state->l;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x56:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->d = state->memory[memory_address];
                    cycles += 7;
                    // state->pc++;
                    break;
                }
                case 0x57:
                {
                    state->d = state->a;
                    cycles += 5;
                    break;
                }
                case 0x58:
                {
                    state->e = state->b;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x59:
                {
                    state->e = state->c;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x5a:
                {
                    state->e = state->d;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x5b:
                {
                    state->e = state->e;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x5c:
                {
                    state->e = state->h;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x5d:
                {
                    state->e = state->l;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x5e:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->e = state->memory[memory_address];
                    cycles += 7;
                    break;
                }
                case 0x5f:
                {
                    state->e = state->a;
                    // uint16_t memory_address = (opcode[2] << 8) | opcode[1];
                    // state->
                    cycles += 5;
                    break;
                }
                case 0x60:
                {
                    state->h = state->b;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x61:
                {
                    state->h = state->c;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x62:
                {
                    state->h = state->d;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x63:
                {
                    state->h = state->e;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x64:
                {
                    state->h = state->h;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x65:
                {
                    state->h = state->l;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x66:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->h = state->memory[memory_address];
                    cycles += 7;
                    break;
                }
                case 0x67:
                {
                    state->h = state->a;
                    cycles += 5;
                    break;
                }
                case 0x68:
                {
                    state->l = state->b;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x69:
                {
                    state->l = state->c;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x6a:
                {
                    state->l = state->d;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x6b:
                {
                    state->l = state->e;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x6c:
                {
                    state->l = state->h;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x6d:
                {
                    state->l = state->l;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x6e:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->l = state->memory[memory_address];
                    cycles += 7;
                    // state->pc++;
                    break;
                }
                case 0x6f:
                {
                    state->l = state->a;
                    cycles += 5;
                    break;
                }
                case 0x70:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->memory[memory_address] = state->b;
                    cycles += 7;
                    // state->pc++;
                    break;
                }
                case 0x71:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->memory[memory_address] = state->c;
                    cycles += 7;
                    // state->pc++;
                    break;
                }
                case 0x72:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->memory[memory_address] = state->d;
                    cycles += 7;
                    // state->pc++;
                    break;
                }
                case 0x73:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->memory[memory_address] = state->e;
                    cycles += 7;
                    // state->pc++;
                    break;
                }
                case 0x74:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->memory[memory_address] = state->h;
                    cycles += 7;
                    // state->pc++;
                    break;
                }
                case 0x75:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->memory[memory_address] = state->l;
                    cycles += 7;
                    // state->pc++;
                    break;
                }
                case 0x76:
                {
                    cycles += 7;
                    break;
                }
                //HLT
                // case 0x76:
                // {
                //     uint16_t memory_address = (opcode[2] << 8) | opcode[1];
                //     state->l = state->memory[memory_address];
                //     state->pc++;
                //     break;
                // }
                case 0x77:
                {
                    uint16_t memory_address = (state->h  << 8) | state->l;
                    state->memory[memory_address] =  state->a;
                    cycles += 7;
                    // state->pc++;
                    break;
                }
                case 0x78:
                {
                    state->a = state->b;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x79:
                {
                    state->a = state->c;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x7a:
                {
                    state->a = state->d;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x7b:
                {
                    state->a = state->e;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x7c:
                {
                    state->a = state->h;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x7d:
                {
                    state->a = state->l;
                    cycles += 5;
                    // state->pc++;
                    break;
                }
                case 0x7e:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->a = state->memory[memory_address];
                    cycles += 7;
                    // state->pc++;
                    break;
                }
                case 0x7f:
                {
                    state->a = state->a;
                    cycles += 5;
                    break;
                }
                case 0x80:
                {
                    state->cc.ac = (((state->a & 0xf) + (state->b & 0xf)) & 0x10) == 0x10;
                    checkA(state->a + state->b, state);
                    state->a = state->a + state->b;
                    cycles += 4;
                    break;
                }
                case 0x81:
                {
                    state->cc.ac = (((state->a & 0xf) + (state->c & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->c;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x82:
                {
                    state->cc.ac = (((state->a & 0xf) + (state->d & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->d;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x83:
                {
                    state->cc.ac = (((state->a & 0xf) + (state->e & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->e;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x84:
                {
                    state->cc.ac = (((state->a & 0xf) + (state->h & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->h;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x85:
                {
                    state->cc.ac = (((state->a & 0xf) + (state->l & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->l;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x86:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->cc.ac = (((state->a & 0xf) + (state->memory[memory_address] & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->memory[memory_address];
                    checkA(state->a, state);
                    cycles += 7;
                    break;
                }
                case 0x87:
                {
                    state->cc.ac = (((state->a & 0xf) + (state->a & 0xf)) & 0x10) == 0x10;
                    uint16_t tmp = state->a + state->a;
                    checkA(tmp, state);
                    state->a = tmp;
                    cycles += 4;
                    break;
                }
                case 0x88:
                {
                    state->cc.ac = (((state->a & 0xf) + ((state->b + state->cc.cy) & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->b + state->cc.cy;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x89:
                {
                    state->cc.ac = (((state->a & 0xf) + ((state->c + state->cc.cy) & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->c + state->cc.cy;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x8a:
                {
                    state->cc.ac = (((state->a & 0xf) + ((state->d + state->cc.cy) & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->d + state->cc.cy;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x8b:
                {
                    state->cc.ac = (((state->a & 0xf) + ((state->e + state->cc.cy) & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->e + state->cc.cy;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x8c:
                {
                    state->cc.ac = (((state->a & 0xf) + ((state->h + state->cc.cy) & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->h + state->cc.cy;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x8d:
                {
                    state->cc.ac = (((state->a & 0xf) + ((state->l + state->cc.cy) & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->l + state->cc.cy;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x8e:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->cc.ac = (((state->a & 0xf) + ((state->memory[memory_address] + state->cc.cy) & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->memory[memory_address] + state->cc.cy;
                    checkA(state->a, state);
                    cycles += 7;
                    break;
                }
                case 0x8f:
                {
                    state->cc.ac = (((state->a & 0xf) + ((state->a + state->cc.cy) & 0xf)) & 0x10) == 0x10;
                    state->a = state->a + state->a + state->cc.cy;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x90:
                {
                    state->cc.ac = ((state->a & 0xf) - (state->b & 0xf)) & 0x10;
                    // state->cc.ac = (((state->a & 0xf) - (state->b & 0xf)) & 0x10) == 0x10; // might have to tack a binary not onto these for subtraction
                    state->a = state->a - state->b;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x91:
                {
                // state->cc.ac = ((state->a & 0xf) - (state->c & 0xf)) & 0x10;
                state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->c) & 0xf) + 1) > 0xf);
                    state->a = state->a - state->c;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x92:
                {
                // state->cc.ac = (((state->a & 0xf) - (state->d & 0xf)) & 0x10);
                state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->d) & 0xf) + 1) > 0xf);
                    state->a = state->a - state->d;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x93:
                {
                // state->cc.ac = (((state->a & 0xf) - (state->e & 0xf)) & 0x10) == 0x10;
                state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->e) & 0xf) + 1) > 0xf);
                    state->a = state->a - state->e;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x94:
                {
                // state->cc.ac = (((state->a & 0xf) - (state->h & 0xf)) & 0x10) == 0x10;
                state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->h) & 0xf) + 1) > 0xf);
                    state->a = state->a - state->h;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x95:
                {
                // state->cc.ac = (((state->a & 0xf) - (state->l & 0xf)) & 0x10) == 0x10;
                state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->l) & 0xf) + 1) > 0xf);
                    state->a = state->a - state->l;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x96:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->memory[memory_address]) & 0xf) + 1) > 0xf);
                // state->cc.ac = (((state->a & 0xf) - (state->memory[memory_address] & 0xf)) & 0x10) == 0x10;
                    state->a = state->a - state->memory[memory_address];
                    checkA(state->a, state);
                    cycles += 7;
                    break;
                }
                case 0x97:
                {
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->a) & 0xf) + 1) > 0xf);
                // state->cc.ac = (((state->a & 0xf) - (state->a & 0xf)) & 0x10) == 0x10;
                    state->a = state->a - state->a;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x98:
                {
                    // state->cc.ac = (((state->a & 0xf) - ((state->b - state->cc.cy) & 0xf)) & 0x10) == 0x10;
                                        state->cc.ac = ((state->a & 0xf) + ((~state->b) & 0xf) + ~state->cc.cy) > 0xf;
                    // state->cc.ac = ((state->a - state->b - state->cc.cy) & 0x10) == 0x10;
                    // state->cc.ac = ((state->a & 0xf) - (state->b & 0xf )- (state->cc.cy & 0xf)) == 0;
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->b) & 0xf) + !state->cc.cy) > 0xf);

                    

                    // state->cc.ac = (unsigned)((state->a & 0xf) + ((~(state->b - state->cc.cy) & 0xf) + 1) > 0xf);
                    checkA(state->a - state->b - state->cc.cy, state);
                    state->a = state->a - state->b - state->cc.cy;
                    cycles += 4;
                    break;
                }
                case 0x99:
                {
                    // printf("state->a: %d, state-l: %d, state->cc.cy: %d\n", state->a, state->l, state->cc.cy);
                    //                     printf("state->a: %d, state-l: %d, state->cc.cy: %d\n", state->a & 0xf, state->l & 0xf, state->cc.cy & 0xf);

                    // state->cc.ac = (((state->a & 0xf) + (state->c & 0xf) + (state->cc.cy & 0xf)) & 0x10) == 0x10;
                                        state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->c) & 0xf) + !state->cc.cy) > 0xf);

                    // state->cc.ac = ((state->a & 0xf) - (state->c & 0xf) - (state->cc.cy & 0xf)) == 0;
                    // state->cc.ac = (unsigned)((state->a & 0xf) + ((~(state->a - state->cc.cy) & 0xf) + 1) > 0xf);
                    state->a = state->a - state->c - state->cc.cy;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x9a:
                {
                // state->cc.ac = (((state->a & 0xf) - ((state->d) & 0xf) - (state->cc.cy & 0xf)) & 0x10) == 0x10;
                    state->cc.ac = ((state->a & 0xf) + ((~state->d) & 0xf) + ~state->cc.cy) > 0xf;
                    state->a = state->a - state->d - state->cc.cy;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x9b:
                {
                // state->cc.ac = (((state->a & 0xf) + ((state->e - state->cc.cy) & 0xf)) & 0x10) == 0x10;
                                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->e) & 0xf) + !state->cc.cy) > 0xf);

                // state->cc.ac = (unsigned)((state->a & 0xf) + ((~(state->e - state->cc.cy) & 0xf) + 1) > 0xf);
                    state->a = state->a - state->e - state->cc.cy;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x9c:
                {
                // state->cc.ac = (((state->a & 0xf) + ((state->h - state->cc.cy) & 0xf)) & 0x10) == 0x10;
                                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->h) & 0xf) + !state->cc.cy) > 0xf);

                // state->cc.ac = (unsigned)((state->a & 0xf) + ((~(state->h - state->cc.cy) & 0xf) + 1) > 0xf);
                    state->a = state->a - state->h - state->cc.cy;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x9d:
                {
                    // printf("state->a: %d, state-l: %d, state->cc.cy: %d\n", state->a, state->l, state->cc.cy);
                    //                     printf("state->a: %d, state-l: %d, state->cc.cy: %d\n", state->a & 0xf, state->l & 0xf, state->cc.cy & 0xf);

                    // state->cc.ac = ((state->a) - (state->l) - (state->cc.cy)) & 0x10;
                    // state->cc.ac =( ((state->a & 0xf) - (~((state->l + state->cc.cy) & 0xf) + 1)) & 0x10) == 0x10;
                    // state->cc.ac = (state->a - (state->l + state->cc.cy));
                    // uint16_t result = state->a + ~state->l + !state->cc.cy;
                    // state->cc.ac = (result ^ state->a ^ state->l ^ state->cc.cy) & 0x10;

                    // state->cc.ac = ((state->a & 0xf) - (state->l & 0xf )- (state->cc.cy & 0xf)) == 0;
                    // state->cc.ac = (state->a ^ state->l ^ state->cc.cy ^ (state->a + state->l + state->cc.cy) & 0x10 ) == 0x10;
                    // state->cc.ac =( (state->a - state->l - state->cc.cy) & 0x10 ) == 0x10;
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->l) & 0xf) + !state->cc.cy) > 0xf);




                    
                    // state->cc.ac = ((state->a) + (~state->l + !state->cc.cy)) & 0x10;

                // state->cc.ac = (((state->a & 0xf) + ((state->l - state->cc.cy) & 0xf)) & 0x10) == 0x10;
                    // state->cc.ac = ((state->a & 0xf) + ((~state->l) & 0xf) + ~state->cc.cy) > 0xf;

                // state->cc.ac = (unsigned)((state->a & 0xf) + ((~(state->l - state->cc.cy) & 0xf) + 1) > 0xf);
                    state->a = state->a - state->l - state->cc.cy;
                    checkA(state->a, state);
                    cycles += 4;
                    break;
                }
                case 0x9e:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                state->cc.ac = (((state->a & 0xf) + ((state->memory[memory_address] - state->cc.cy) & 0xf)) & 0x10) == 0x10;
                    checkA(state->a - state->memory[memory_address] - state->cc.cy, state);
                    state->a = state->a - state->memory[memory_address] - state->cc.cy;
                    cycles += 7;
                    break;
                }
                case 0x9f:
                {
                // state->cc.ac = (((state->a & 0xf) + ((state->a - state->cc.cy) & 0xf)) & 0x10) == 0x10;
                                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->a) & 0xf) + !state->cc.cy) > 0xf);

                    state->a = state->a - state->a - state->cc.cy;
                                        // printf("state->a: %d\n", state->a);

                    checkA(state->a - state->a - state->cc.cy, state);

                    cycles += 4;
                    break;
                }
                case 0xa0:
                {
                    // state->cc.ac = (((state->a & 0xf) & (state->b & 0xf)) & 0x10) == 0x10;
                    state->a = state->a & state->b;
                    checkOpA(state);
                    // state->cc.ac = (((state->a & 0xf) + ((state->a - state->cc.cy) & 0xf) & 0x10) == 0x10);
                    cycles += 4;
                    break;
                }
                case 0xa1:
                {
                    // state->cc.ac = (((state->a & 0xf) & (state->c & 0xf)) & 0x10) == 0x10;
                    state->a = state->a & state->c;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xa2:
                {
                    // state->cc.ac = (((state->a & 0xf) ^ (state->d & 0xf)) & 0x10) == 0x10;
                    state->a = state->a & state->d;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xa3:
                {
                    // state->cc.ac = (((state->a & 0xf) & (state->e & 0xf)) & 0x10) == 0x10;
                    state->a = state->a & state->e;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xa4:
                {
                    // state->cc.ac = (((state->a & 0xf) & (state->h & 0xf)) & 0x10) == 0x10;
                    state->a = state->a & state->h;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xa5:
                {
                    // state->cc.ac = (((state->a & 0xf) & (state->l & 0xf)) & 0x10) == 0x10;
                    state->a = state->a & state->l;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xa6:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    // state->cc.ac = (((state->a & 0xf) & (state->memory[memory_address] & 0xf)) & 0x10) == 0x10;
                    state->a = state->a & state->memory[memory_address];
                    checkOpA(state);
                    cycles += 7;
                    break;
                }
                case 0xa7:
                {
                    // state->cc.ac = (((state->a & 0xf) & (state->a & 0xf)) & 0x10) == 0x10;
                    state->a &= state->a;
                    checkOpA(state);
                    cycles += 4;
                    // delete this 
                    state->cc.ac = 0;
                    break;
                }
                case 0xa8:
                {
                    state->cc.ac = (((state->a & 0xf) ^ (state->b & 0xf)) & 0x10) == 0x10;
                    state->a = state->a ^ state->b;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xa9:
                {
                    state->cc.ac = (((state->a & 0xf) ^ (state->c & 0xf)) & 0x10) == 0x10;
                    state->a = state->a ^ state->c;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xaa:
                {
                    state->cc.ac = (((state->a & 0xf) ^ (state->d & 0xf)) & 0x10) == 0x10;
                    state->a = state->a ^ state->d;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xab:
                {
                    state->cc.ac = (((state->a & 0xf) ^ (state->e & 0xf)) & 0x10) == 0x10;
                    state->a = state->a ^ state->e;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xac:
                {
                    state->cc.ac = (((state->a & 0xf) ^ (state->h & 0xf)) & 0x10) == 0x10;
                    state->a = state->a ^ state->h;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xad:
                {
                    state->cc.ac = (((state->a & 0xf) ^ (state->l & 0xf)) & 0x10) == 0x10;
                    state->a = state->a ^ state->l;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xae:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->cc.ac = (((state->a & 0xf) ^ (state->memory[memory_address] & 0xf)) & 0x10) == 0x10;
                    state->a = state->a ^ state->memory[memory_address];
                    checkOpA(state);
                    cycles += 7;
                    break;
                }
                case 0xaf:
                {
                    state->cc.ac = (((state->a & 0xf) ^ (state->a & 0xf)) & 0x10) == 0x10;
                    state->a ^= state->a;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xb0:
                {
                    state->cc.ac = (((state->a & 0xf) | (state->b & 0xf)) & 0x10) == 0x10;
                    state->a = state->a | state->b;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xb1:
                {
                    state->cc.ac = (((state->a & 0xf) | (state->c & 0xf)) & 0x10) == 0x10;
                    state->a = state->a | state->c;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xb2:
                {
                    state->cc.ac = (((state->a & 0xf) | (state->d & 0xf)) & 0x10) == 0x10;
                    state->a = state->a | state->d;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xb3:
                {
                    state->cc.ac = (((state->a & 0xf) | (state->e & 0xf)) & 0x10) == 0x10;
                    state->a = state->a | state->e;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xb4:
                {
                    state->cc.ac = (((state->a & 0xf) | (state->h & 0xf)) & 0x10) == 0x10;
                    state->a = state->a | state->h;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xb5:
                {
                    state->cc.ac = (((state->a & 0xf) | (state->l & 0xf)) & 0x10) == 0x10;
                    state->a = state->a | state->l;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xb6:
                {
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->cc.ac = (((state->a & 0xf) | (state->memory[memory_address] & 0xf)) & 0x10) == 0x10;
                    state->a = state->a | state->memory[memory_address];
                    checkOpA(state);
                    cycles += 7;
                    break;
                }
                case 0xb7:
                {
                    state->cc.ac = (((state->a & 0xf) | (state->a & 0xf)) & 0x10) == 0x10;
                    state->a = state->a | state->a;
                    checkOpA(state);
                    cycles += 4;
                    break;
                }
                case 0xb8:
                {
                    // state->cc.ac = (((state->a & 0xf) - (state->b & 0xf) & 0x10) == 0x10);
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->b) & 0xf) + 1) > 0xf);
                    // printf("state->a: %d, state->b: %d\n", state->a, state->b);
                    // state->cc.ac = ((state->a & 0xf) - (state->b & 0xf)) & 0x10;
                    // state->a -= state->b;
                    // state->a -= state->b;
                    uint16_t result = state->a - state->b;
                    // These are supposed to compare not set -=
                    checkA(result, state);
                    cycles += 4;
                    break;
                }
                case 0xb9:
                {
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->c) & 0xf) + 1) > 0xf);
                    uint16_t result = state->a - state->c;
                    // state->cc.ac = (((state->a & 0xf) + (state->b & 0xf) & 0x10) == 0x10);
                
                    checkA(result, state);
                    cycles += 4;
                    break;
                }
                case 0xba:
                {
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->d) & 0xf) + 1) > 0xf);
                    uint16_t result = state->a - state->d;
                    // state->cc.ac = (((state->a & 0xf) + (state->b & 0xf) & 0x10) == 0x10);
                    checkA(result, state);
                    cycles += 4;
                    break;
                }
                case 0xbb:
                {
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->e) & 0xf) + 1) > 0xf);
                    uint16_t result = state->a - state->e;
                // state->cc.ac = (((state->a & 0xf) + (state->b & 0xf) & 0x10) == 0x10);
                    checkA(result, state);
                    cycles += 4;
                    break;
                }
                case 0xbc:
                {
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->h) & 0xf) + 1) > 0xf);
                    uint16_t result = state->a - state->h;
                // state->cc.ac = (((state->a & 0xf) + (state->b & 0xf) & 0x10) == 0x10);
                    checkA(result, state);
                    cycles += 4;
                    break;
                }
                case 0xbd:
                {
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->l) & 0xf) + 1) > 0xf);
                    uint16_t result = state->a - state->l;
                // state->cc.ac = (((state->a & 0xf) + (state->b & 0xf) & 0x10) == 0x10);
                    checkA(result, state);
                    cycles += 4;
                    break;
                }
                case 0xbe:
                {
                    
                    uint16_t memory_address = (state->h << 8) | state->l;
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->memory[memory_address]) & 0xf) + 1) > 0xf);
                    uint16_t result = state->a - state->memory[memory_address];
                // state->cc.ac = (((state->a & 0xf) + (state->b & 0xf) & 0x10) == 0x10);
                    checkA(result, state);
                    cycles += 7;
                    break;
                }
                case 0xbf:
                {
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->a) & 0xf) + 1) > 0xf);
                    uint16_t result = state->a - state->a;
                state->cc.ac = (((state->a & 0xf) + (state->b & 0xf) & 0x10) == 0x10);
                    checkA(result, state);
                    cycles += 4;
                    break;
                }
                case 0xc0:
                {
                    // NOTE There is a good chance that these might not work
                    if (state->cc.z == 0)
                    {
                        state->pc = (state->memory[state->sp+1] << 8) | state->memory[state->sp];
                        // state->pc = (state->memory[state->pc + 1] << 8) | state->memory[state->pc];
                        state->sp += 2;
                        return 11;
                        // cycles += 11; /// Note might have to move this outside of the if statement MAY
                    }
                    cycles += 11;
                    // cycles += 5;
                    break;
                } 
                case 0xc1:
                {
                    //POP
                    state->c = state->memory[state->sp];
                    state->b = state->memory[state->sp + 1];
                    state->sp = state->sp + 2;
                    cycles += 10;
                    break;
                }
                case 0xc2:
                {
                    //JNZ
                    if (state->cc.z == 0)
                    {
                        state->pc = (opcode[2] << 8) | opcode[1];
                    }
                    else
                    {
                        state->pc += 2;
                    }
                    cycles += 10;
                    break;
                }
                case 0xc3:
                {
                    //JMP
                    state->pc = (opcode[2] << 8) | opcode[1];
                    cycles += 10;
                    break;
                }
                case 0xc4:
                {
                    // *** 
                    // NOTE Again this is probably not going to work check it I think i might not know what the if nz part really means
                    // if (state->cc.z == 0)
                    // {
                    //     uint16_t ret = state->pc + 2;
                    //     state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    //     state->memory[state->sp - 2] = (ret & 0xff);
                    //     state->sp = state->sp - 2;
                    //     state->pc = (opcode[2] << 8) | opcode[1];
                    // }
                    // else
                    // {
                    //     // cycles += 11;
                    //     state->pc + 2;
                    // }
                    // cycles += 17;
                    if (state->cc.z  == 0) {
                        call(state, opcode, &cycles);
                        return 17;
                    }
                    else {
                        state->pc += 2;
                    }
                    break;
                }
                case 0xc5:
                {
                    //PUSH
                    state->sp = state->sp - 2;
                    state->memory[state->sp + 1] = state->b;
                    state->memory[state->sp] = state->c;
                    cycles += 11;
                    break;
                }
                case 0xc6: // NOTE breaks here for real
                {
                    //ADI
                    // printf("a: %d + opcode: %d\n", state->a, opcode[1]);
                    uint16_t tmp = state->a + opcode[1];
                    // uint16_t tmp = (uint16_t) state->a + (uint16_t) opcode[1];
                    checkZSP(tmp&0xff, state);
                    state->cc.cy = (tmp > 0xff);
                    state->cc.ac = ((state->a & 0x0f)  + (opcode[1] & 0x0f) > 0xf);
                    // state->a = tmp&0xff;
                    state->a += opcode[1];
                    state->pc += 1;
                    cycles += 7;
                    break;
                }
                case 0xc7:
                {
                    uint16_t ret = state->pc + 2;
                    state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    state->memory[state->sp - 2] = (ret & 0xff);
                    state->sp = state->sp - 2;
                    state->pc = 0x0;
                    cycles += 11;
                    break;
                    //RST 0
                    // //NOTE this might not work
                    // state->sp = 0;
                    // // state->pc = 0;
                    // break;

                }
                case 0xc8:
                {
                    // *** 
                    if (state->cc.z)
                    {
                        state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
                        state->sp += 2;
                        return 11;
                    }
                    cycles += 11;
                    // CHECK THIS NOWWWWW
                    // cycles += 5;
                    break;
                }
                case 0xc9:
                {
                    //RET
                    state->pc = state->memory[state->sp] | (state->memory[state->sp + 1] << 8);    
                    state->sp += 2;
                    cycles += 10;
                    break;
                }
                case 0xca:
                {
                    if (state->cc.z)
                    {
                        state->pc = (opcode[2] << 8) | opcode[1];
                    }
                    else
                    {
                        state->pc += 2;
                    }
                    cycles += 10;
                    break;
                }
                case 0xcb:
                {
                    cycles += 10;
                    break;
                }
                case 0xcc:
                {
                    // if (state->cc.z)
                    // {
                    //     uint16_t ret = state->pc + 2;
                    //     state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    //     state->memory[state->sp - 2] = (ret & 0xff);
                    //     state->sp = state->sp - 2;
                    //     state->pc = (opcode[2] << 8) | opcode[1];
                    // }
                    // else
                    // {
                    //     state->pc + 2;
                    //     // cycles += 11;
                    // }
                    // cycles += 17;
                    if (state->cc.z == 1) {
                        call(state, opcode, &cycles);
                        return 17;
                    }
                    else {
                        state->pc += 2;
                    }
                    break;
                }
                case 0xcd:
                {
                    // This is the CP/M opcode for debugging
                    if (!call(state, opcode, &cycles)) {
                        state->pc += 2;
                    }
                    else {
                        return 17;
                    }
                    
                    break;
                }
                case 0xce:
                {
                    // NOTE This probably is old code that does not work reimplement after testing.
                    // state->memory[state->sp-1] = state->pc >> 8;
                    // state->memory[state->sp-2] = state->pc & 0xff;
                    // state->sp -= 2;
                    // state->pc = (opcode[2] << 8) | opcode[1];
                    // cycles += 7;
                    //NOTE This might be broken due to the way flags are handled MAY
                    uint16_t tmp = state->a + opcode[1] + state->cc.cy;
                    checkZSP(tmp & 0xff, state);
                    state->cc.cy = (tmp > 0xff);
                    state->cc.ac = ((state->a & 0x0f)  + ((opcode[1]  + state->cc.cy) & 0x0f) > 0xf);
                    state->a = tmp & 0xff;
                    state->pc++;
                    cycles += 7;
                    break;
                }
                case 0xcf:
                {
                    uint16_t ret = state->pc + 2;
                    state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    state->memory[state->sp - 2] = (ret & 0xff);
                    state->sp = state->sp - 2;
                    state->pc = 0x0008;
                    cycles += 11;
                    break;
                    // state->sp = 8;
                }
                case 0xd0:
                {
                    if (state->cc.cy == 0)
                    {
                        state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
                        state->sp += 2;
                        return 11;
                    }
                    cycles += 11;
                    // cycles += 5;
                    break;

                }
                case 0xd1:
                {
                    //POP
                    state->e = state->memory[state->sp];
                    state->d = state->memory[state->sp + 1];
                    state->sp = state->sp + 2;
                    cycles += 10;
                    break;
                }
                case 0xd2:
                {
                    // NOTE this probably won't work
                    if (state->cc.cy == 0)
                    {
                        state->pc = (opcode[2] << 8) | opcode[1];
                    }
                    else
                    {
                        state->pc += 2;
                    }
                    cycles += 10;
                    break;
                }
                case 0xd3:
                {
                    //OUT
                    // uint8_t port = state->memory[state->pc + 1];
                    // uint16_t port = state->memory[state->pc];
                    uint8_t port = opcode[1];
                    machineOUT(port, state);
                    state->pc += 1;
                    cycles += 3;
                    // printf("OUT: %d\n", port);
                    // cycles += 10;
                    break;
                }
                case 0xd4:
                {
                    // if (!state->cc.cy) // might be state->cc.cy == 0
                    // {
                    //     uint16_t ret = state->pc + 2;    
                    //     state->memory[state->sp - 1] = ret >> 8;    
                    //     state->memory[state->sp - 2] = ret;    
                    //     state->sp = state->sp - 2;    
                    //     state->pc = (opcode[2] << 8) | opcode[1];
                    // }
                    // else {
                    //     // cycles += 11;
                    //     state->pc += 2;
                    // }
                    // cycles += 17;
                    if (state->cc.cy == 0) {
                        call(state, opcode, &cycles);
                        return 17;
                    }else {
                        state->pc += 2;
                    }
                    break;
                }
                case 0xd5:
                {
                    //PUSH
                    state->sp = state->sp - 2;
                    state->memory[state->sp + 1] = state->d;
                    state->memory[state->sp] = state->e;
                    cycles += 11;
                    break;
                }
                case 0xd6:
                {
                    uint8_t tmp = state->a - opcode[1];
                    checkZSP(tmp, state);
                    state->cc.cy = (state->a < opcode[1]);
                    // state->cc.ac = ((state->a & 0x0f)  - (opcode[1] & 0x0f) < 0xf);
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~opcode[1]) & 0xf) + 1) > 0xf);
                    state->a = tmp;
                    state->pc++;
                    cycles += 7;
                    break;
                }
                case 0xd7:
                {
                    // ***
                    uint16_t ret = state->pc + 2;
                    state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    state->memory[state->sp - 2] = (ret & 0xff);
                    state->sp = state->sp - 2;
                    state->pc = 0x10;
                    cycles += 11;
                    break;
                    //NOTE this probably will not work
                    // state->sp = 10;
                    // break;
                }
                case 0xd8:
                {
                    // NOTE this probably won't work
                    // ***
                    if (state->cc.cy != 0)
                    {
                        state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
                        state->sp += 2;
                        return 11;
                    }
                    cycles += 11;
                    // cycles += 5;

                    break;
                }
                case 0xd9:
                {
                    cycles += 10;
                    break;
                }
                case 0xda:
                {
                    if (state->cc.cy != 0)
                    {
                        state->pc = (opcode[2] << 8) | opcode[1];
                    }
                    else {
                        state->pc += 2;
                    }
                    cycles += 10;
                    break;
                }
                case 0xdb:
                {
                    //IN
                    // uint8_t port = state->memory[state->pc ];
                    uint8_t port = opcode[1];
                    state->a = machineIN(*state, port);
                    printf("IN: %d\n", state->a);
                    cycles += 3;// used to be 10 but other guy has 3
                    state->pc++; // might have to modify this.
                    break;
                }
                case 0xdc:
                {
                    // //NOTE THIS MIGHT NOT WORK
                    // if (state->cc.cy)
                    // {
                    //     uint16_t ret = state->pc + 2;
                    //     state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    //     state->memory[state->sp - 2] = (ret & 0xff);
                    //     state->sp = state->sp - 2;
                    //     state->pc = (opcode[2] << 8) | opcode[1];
                    // }
                    // else
                    // {
                    //     // cycles += 11;
                    //     state->pc += 2;
                    // }
                    // cycles += 17;
                    // break;
                    if (state->cc.cy != 0) {
                        call(state, opcode, &cycles);
                        return 17;
                    }
                    else {
                        state->pc += 2;
                    }
                    break;
                }
                case 0xdd:
                {
                    cycles += 17; // NOTE might be wrong number of cycles;
                    break;
                }
                case 0xde:
                {
                    //NOTE This might not work
                    // *** 
                    uint16_t tmp = state->a - opcode[1] - state->cc.cy;
                    checkZSP(tmp&0xff, state);
                    state->cc.cy = (tmp > 0xff);
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~opcode[1]) & 0xf) + 1) > 0xf);
                    state->a = tmp & 0xff;
                    state->pc++;
                    cycles += 7;
                    break;
                }
                case 0xdf:
                {
                    //note this might not work
                    // ***
                    uint16_t ret = state->pc + 2;
                    state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    state->memory[state->sp - 2] = (ret & 0xff);
                    state->sp = state->sp - 2;
                    state->pc = 0x18;
                    cycles += 11;
                    break;
                }
                case 0xe0:
                {
                    if (state->cc.p == 0)
                    {
                        state->pc = (state->memory[state->sp + 1]) << 8 | state->memory[state->sp];
                        state->sp += 2;
                        return 11;
                    }
                    cycles += 11;
                    // cycles += 5;
                    break;
                }
                case 0xe1:
                {
                    //POP
                    // printf("l %d, h %d\n", state->memory[state->sp ], state->memory[state->sp + 1]);
                    state->l = state->memory[state->sp];
                    state->h = state->memory[state->sp + 1];
                    state->sp = state->sp + 2;
                    cycles += 10;
                    break;
                }
                case 0xe2:
                {
                    //JMP
                    if (state->cc.p == 0)
                    {
                        state->pc = (opcode[2] << 8) | opcode[1];
                    }
                    else
                    {
                        state->pc += 2;
                    }
                    cycles += 10;
                    break;
                }
                case 0xe3:
                {
                    uint8_t buffer;
                    buffer = state->l;
                    state->l = state->memory[state->sp];
                    state->memory[state->sp] = buffer;
                    buffer = state->h;
                    state->h = state->memory[(state->sp+1)];
                    state->memory[(state->sp+1)]= buffer;
                    cycles += 18;
                    break;
                }
                case 0xe4:
                {
                    // if (state->cc.p == 0)
                    // {
                    //     uint16_t ret = state->pc + 2;
                    //     state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    //     state->memory[state->sp - 2] = (ret & 0xff);
                    //     state->sp = state->sp - 2;
                    //     state->pc = (opcode[2] << 8) | opcode[1];
                    // }
                    // else
                    // {
                    //     // cycles += 11;
                    //     state->pc += 2;
                    // }
                    // cycles += 17;
                    if (state->cc.p == 0) {
                        call(state, opcode, &cycles);
                        return 17;
                    }
                    else {
                        state->pc += 2;
                    }
                    break;
                }
                case 0xe5:
                {
                    //PUSH
                    state->sp = state->sp - 2;
                    state->memory[state->sp + 1] = state->h;
                    state->memory[state->sp] = state->l;
                    cycles += 11;
                    break;
                }
                case 0xe6:
                {
                    //ANI
                    // printf("test");
                    // printf("AX: %d\n", state->cc.ac);
                    // uint16_t test = state->a;
                    state->cc.ac = (((state->a & 0xf) + (opcode[1] & 0xf)) & 0x10) == 0x10;
                    state->a &= opcode[1];
                    // state->cc.ac = 1;
                    // state->a = state->a & opcode[1]; old
                    checkOpA(state);
                    state->pc += 1;
                    cycles += 7;
                    break;
                }
                case 0xe7:
                {
                    uint16_t ret = state->pc + 2;
                    state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    state->memory[state->sp - 2] = (ret & 0xff);
                    state->sp = state->sp - 2;
                    state->pc = 0x20;
                    cycles += 11;
                    break;
                }
                case 0xe8:
                {
                    if (state->cc.p)
                    {
                        state->pc = (state->memory[state->sp + 1]) << 8 | state->memory[state->sp];
                        state->sp += 2;
                        return 11;
                    }
                    cycles += 11;
                    // cycles += 5;
                    break;
                }
                case 0xe9:
                {
                    state->pc = (state->h << 8) | state->l;
                    cycles += 5;
                    break;
                }
                case 0xea:
                {
                    if (state->cc.p == 1)
                    {
                        state->pc = (opcode[2] << 8) | opcode[1];
                    }
                    else
                    {
                        state->pc += 2;
                    }
                    cycles += 10;
                    break;
                }
                case 0xeb:
                {
                    //XCHG
                    uint8_t buffer;
                    buffer = state->h;
                    state->h = state->d;
                    state->d = buffer;
                    buffer = state->l;
                    state->l = state->e;
                    state->e = buffer;
                    cycles += 5;
                    break;
                }
                case 0xec:
                {
                    // if (state->cc.p != 0)
                    // {
                    //     uint16_t ret = state->pc + 2;
                    //     state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    //     state->memory[state->sp - 2] = (ret & 0xff);
                    //     state->sp = state->sp - 2;
                    //     state->pc = (opcode[2] << 8) | opcode[1];
                    // }
                    // else
                    // {
                    //     state->pc += 2;
                    //     // cycles += 11;
                    // }
                    // cycles += 17;
                    if (state->cc.p != 0) {
                        call(state, opcode, &cycles);
                        return 17;
                    }
                    else {
                        state->pc += 2;
                    }
                    break;
                }
                case 0xed:
                {
                    cycles += 17; //NOTE Might be wrong number of cycles
                    break;
                }
                case 0xee:
                {
                    // uint8_t tmp = state->a ^ opcode[1];
                    // checkZSP(tmp, state);
                    // // state->cc.cy = 0;
                    // // state->cc.cy = (tmp > 0xff);
                    // state->a = tmp;
                    // cycles += 7;
                    // printf("a: %d\n opcode: %d\n", state->a, opcode[1]);
                    state->a ^= opcode[1];
                    state->cc.ac = 0;
                    checkOpA(state);
                    // state->cc.cy = 0;
                    // state->cc.ac = ((state->a & 0x0f)  ^ (opcode[1] & 0x0f) > 0xf);
                    // state
                    state->pc++;
                    // checkZSP(state->a, state);
                    break;
                }
                case 0xef:
                {
                    uint16_t ret = state->pc + 2;
                    state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    state->memory[state->sp - 2] = (ret & 0xff);
                    state->sp = state->sp - 2;
                    state->pc = 0x28;
                    cycles += 11;
                    break;
                }
                case 0xf0:
                {
                    if (state->cc.s == 0) // this was the sign bit in the other guys code was parity bit
                    {
                        state->pc = (state->memory[state->sp + 1]) << 8 | state->memory[state->sp];
                        state->sp += 2;
                        return 11;
                    }
                    cycles += 11;
                    // cycles += 5;
                    break;
                }
                case 0xf1:
                {
                    //POP
                    // ***  Definately old
                    state->a = state->memory[state->sp + 1];
                    uint8_t psw = state->memory[state->sp];

                    state->cc.s = psw >> 7;
                    state->cc.z = (psw >> 6) & 0x1;
                    state->cc.ac = (psw >> 4) & 0x1;
                    state->cc.p = (psw >> 2) & 0x1;
                    state->cc.cy = psw & 0x1;
                    // state->cc = state->memory[state->sp];
                    // state->cc.z  = (0x01 == (psw & 0x01));
                    // state->cc.s  = (0x02 == (psw & 0x02));
                    // state->cc.s = (psw << 1) & 1;
                    // state->cc.s = (0x80 == (psw & 0x80));
                    // state->cc.p  = (0x04 == (psw & 0x04));
                    // state->cc.cy = (0x08 == (psw & 0x08));
                    // state->cc.ac = (0x10 == (psw & 0x10));
                    state->sp += 2;
                    cycles += 10;
                    break;
                }
                case 0xf2:
                {   
                    // NOTE might have to be state->cc.p == 0 or 1 idk was 2 before i changed it;
                    if (state->cc.s == 0) // in other guys code this was sign == 0 was p  == 2
                    {
                        state->pc = (opcode[2] << 8) | opcode[1];
                    }
                    else
                    {
                        state->pc += 2;
                    }
                    cycles += 10;
                    break;
                }
                case 0xf3:
                {
                    state->int_enable = 0;
                    cycles += 4;
                    break;
                }
                case 0xf4:
                {
                    // originally had no == 0
                    // if (state->cc.p == 0)
                    // {
                    //     // Note TODAY this is probably wrong as it differs from the other guys code so change if it does not work
                    //     uint16_t ret = state->pc + 2;
                    //     state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    //     state->memory[state->sp - 2] = (ret & 0xff);
                    //     state->sp = state->sp - 2;
                    //     state->pc = (opcode[2] << 8) | opcode[1];
                    // }
                    // else
                    // {
                    //     state->pc += 2;
                    //     // cycles += 11;
                    // }
                    // cycles += 17;
                    if (state->cc.s == 0) {
                        call(state, opcode, &cycles);
                        return 17;
                    }
                    else {
                        state->pc += 2;
                    }
                    break;
                }
                case 0xf5:
                {
                    //PUSH
                    state->sp = state->sp - 2;
                    state->memory[state->sp + 1] = state->a;
                    uint8_t psw = 0;
                    // state->memory[state->sp - 2] = *(unsigned char *) &state->cc;
                    psw |= state->cc.s << 7;
                    psw |= state->cc.z << 6;
                    psw |= state->cc.ac << 4;
                    psw |= state->cc.p << 2;
                    psw |= 1 << 1;
                    psw |= state->cc.cy << 0;
                    // uint8_t psw = (state->cc.z |
                    //                 state->cc.s << 1 |
                    //                 state->cc.p << 2 |
                    //                 state->cc.cy << 3 |
                    //                 state->cc.ac << 4 );
                    state->memory[state->sp] = psw;
                    cycles += 11;
                    break;
                }
                case 0xf6:
                {
                    uint8_t tmp = state->a | opcode[1];
                    checkZSP(tmp, state);
                    state->cc.cy = 0;
                    state->cc.ac = ((state->a & 0x0f)  | (opcode[1] & 0x0f) > 0xf);
                    state->a = tmp;
                    cycles += 7;
                    state->pc++;
                    break;
                }
                case 0xf7:
                {
                    uint16_t ret = state->pc + 2;
                    state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    state->memory[state->sp - 2] = (ret & 0xff);
                    state->sp = state->sp - 2;
                    state->pc = 0x30;
                    cycles += 11;
                    break;
                }
                case 0xf8:
                {
                    if (state->cc.s != 0)
                    {
                        state->pc = (state->memory[state->sp + 1]) << 8 | state->memory[state->sp];
                        state->sp += 2;
                        return 11;
                    }
                    // cycles += 5;
                    cycles += 11;
                    break;
                }
                case 0xf9:
                {
                    state->sp = (state->h << 8) | state->l;
                    cycles += 5;
                    break;
                }
                case 0xfa:
                {
                    // JM adr
                    if (state->cc.s != 0)
                    {
                        state->pc = (opcode[2] << 8) | opcode[1];
                    }
                    else {
                        state->pc += 2;
                    }
                    cycles += 10;
                    break;
                }
                case 0xfb:
                {
                    //EI
                    state->cc.interrupt_enabled = 1;
                    // printf("enabling interrupt\n");
                    cycles += 4;
                    break;
                }
                case 0xfc:
                {
                    // if (state->cc.s != 0)
                    // {
                    //     uint16_t ret = state->pc + 2;
                    //     state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    //     state->memory[state->sp - 2] = (ret & 0xff);
                    //     state->sp = state->sp - 2;
                    //     state->pc = (opcode[2] << 8) | opcode[1];
                    // }
                    // else
                    // {
                    //     state->pc += 2;
                    //     // cycles += 11;
                    // }
                    // cycles += 17;
                    if (state->cc.s != 0) {
                        call(state, opcode, &cycles);
                        return 17;
                    }
                    else {
                        state->pc += 2;
                    }
                    break;
                }
                case 0xfd:
                {
                    cycles += 17;
                    break;
                }
                case 0xfe:
                {
                    //CPI
                    uint16_t tmp = state->a - opcode[1];
                    // printf("state->a: %d\n opcode[1]: %d\n", state->a, opcode[1]);

                    checkZSP(tmp&0xff, state);
                    state->cc.cy = (state->a < opcode[1]); //NOTE was tmp < opcode[1]
                    state->cc.ac = (unsigned)((state->a & 0xf) + (((~opcode[1]) & 0xf) + 1) > 0xf);
                    state->pc += 1;
                    cycles += 7;
                    break;
                }
                case 0xff: {
                    uint16_t ret = state->pc + 2;
                    state->memory[state->sp - 1] = (ret >> 8) & 0xff;
                    state->memory[state->sp - 2] = (ret & 0xff);
                    state->sp = state->sp - 2;
                    state->pc = 0x38;
                    cycles += 11;
                    break;
                }
            }
            // case 0xff: UnimplementedInstruction(state); break;
            // default:
            // {
            //     UnimplementedInstruction(state); break;
            // }
        // }
    // return cycles;
    // return cycles8080[*opcode];
    if (*opcode == 0xe4) {
        
    }
    return OPCODES_CYCLES[*opcode];
       
    
}