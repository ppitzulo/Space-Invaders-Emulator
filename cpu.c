#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "disassembler.h"
#include "cpu.h"

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


void UnimplementedInstruction(State8080* state)
{
    //pc will have advanced one, so undo that
    printf("Error: Unimplemented instruction\n");
    printf("%d", state->memory[state->pc]);
    exit(1);
}


int Emulate8080Op(State8080* state)
{
    int cycles = 0;

    while (cycles < 33333)
    {
        unsigned char *opcode = &state->memory[state->pc];
        Disassemble8080Op(state->memory, state->pc);
        
        state->pc++;
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
                uint16_t pair = (state->b<<8) | state->c;
                pair++;
                state->b = pair >> 8;
                state->c = pair;
                cycles += 5;
                break;
            }
            case 0x04:
            {
                //INR
                state->b++;
                CheckFlags(state->b, state);
                cycles += 5;
                break;
            }
            case 0x05:
            {
                //DCR
                state->b--;
                CheckFlags(state->b, state);
                cycles += 5;
                break;
            }
            case 0x06:
            {
                //MVI
                state->b = opcode[1];
                state->pc += 1;
                cycles += 7;
                break;
            }
            case 0x09:
            {
                //DAD
                uint16_t BC = (state->b<<8) | state->c;
                uint16_t HL = (state->h<<8) | state->l;
                HL += BC;
                state->h = HL>>8;
                state->l = HL;
                CheckFlags(HL, state);
                cycles += 10;
                break;
            }
            case 0x0d:
            {
                //DCR
                state->c--;
                CheckFlags(state->c, state);
                cycles += 10;
                break;
            }
            case 0x0e:
            {
                //MVI
                state->c = opcode[1];
                state->pc += 1;
                cycles += 7;
                break;
            }
            case 0x0f:
            {
                //RRC
                state->cc.cy = (1 == (state->a & 1));
                state->a = ((state->a & 1) << 7) | (state->a >> 1);
                CheckFlags(state->a, state);
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
            case 0x13:
            {
                //INX
                uint16_t pair = (state->d << 8) | state->e;
                pair++;
                state->d = pair >> 8;
                state->e = pair;
                cycles += 5;
                break;
            }
            case 0x19:
            {
                //DAD
                uint16_t DE = (state->d << 8) | state->e;
                uint16_t HL = (state->h << 8) | state->l;
                HL += DE;
                state->h = HL >> 8;
                state->l = HL;
                CheckFlags(HL, state);
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
            case 0x21:
            {
                //LXI
                state->l = opcode[1];
                state->h = opcode[2];
                state->pc += 2;
                cycles += 10;
                break;
            }
            case 0x23:
            {
                //INX
                uint16_t pair = (state->h << 8) | state->l;
                pair++;
                state->h = pair >> 8;
                state->l = pair;
                cycles += 5;
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
            case 0x29:
            {
                //DAD
                uint16_t HL = (state->h << 8) | state->l;
                HL *= 2;
                state->h = HL >> 8;
                state->l = HL;
                CheckFlags(HL, state);
                cycles += 10;
                break;
            }
            case 0x31:
            {
                //LXI
                uint8_t high = state->sp >> 8;
                uint8_t low = state->sp;
                low = opcode[1];
                high = opcode[2];
                state->sp = (high << 8) | low;
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
            case 0x36:
            {
                //MVI
                uint16_t HL = (state->h << 8) | state->l;
                state->memory[HL] = opcode[1];
                state->pc += 1;
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
            case 0x3e:
            {
                //MVI
                state->a = opcode[1];
                state->pc += 1;
                cycles += 7;
                break;
            }
            case 0x56:
            {
                //MOV
                uint16_t HL = (state->h << 8) | state->l;
                state->d = state->memory[HL];
                cycles += 7;
                break;
            }
            case 0x5e:
            {
                //MOV
                uint16_t HL = (state->h << 8) | state->l;
                state->e = state->memory[HL];
                cycles += 7;
                break;
            }
            case 0x66:
            {
                //MOV
                uint16_t HL = (state->h << 8) | state->l;
                state->h = state->memory[HL];
                cycles += 7;
                break;
            }
            case 0x6f:
            {
                //MOV
                state->l = state->a;
                cycles += 5;
                break;
            }
            case 0x77:
            {
                //MOV
                uint16_t HL = (state->h << 8) | state->l;
                state->memory[HL] = state->a;
                cycles += 7;
                break;
            }
            case 0x7a:
            {
                //MOV
                state->a = state->d;
                cycles += 5;
                break;
            }
            case 0x7b:
            {
                //MOV
                state->a = state->e;
                cycles += 5;
                break;
            }
            case 0x7c:
            {
                //MOV
                state->a = state->h;
                cycles += 5;
                break;
            }
            case 0x7e:
            {
                //MOV 
                uint16_t HL = (state->h << 8) | state->l;
                state->a = state->memory[HL];
                cycles += 7;
                break;
            }
            case 0xa7:
            {
                //ANA
                state->a &= state->a;
                CheckFlags(state->a, state);
                cycles += 4;
                break;
            }
            case 0xaf:
            {
                //XRA
                state->a ^= state->a;
                CheckFlags(state->a, state);
                cycles += 4;
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
                uint16_t memory_address = (opcode[2] << 8) | opcode[1];
                if (state->cc.z)
                {
                    state->pc += 2;
                }
                else
                {
                    state->pc = memory_address;
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
            case 0xc5:
            {
                //PUSH
                state->memory[state->sp - 1] = state->b;
                state->memory[state->sp - 2] = state->c;
                state->sp = state->sp - 2;
                cycles += 11;
                break;
            }
            case 0xc6:
            {
                //ADI
                state->a += opcode[1];
                CheckFlags(state->a, state);
                state->pc += 1;
                cycles += 7;
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
            case 0xcd:
            {
                //CALL
                uint16_t    ret = state->pc + 2;    
                state->memory[state->sp - 1] = ret >> 8;    
                state->memory[state->sp - 2] = ret;    
                state->sp = state->sp - 2;    
                state->pc = (opcode[2] << 8) | opcode[1];
                cycles += 17;
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
            case 0xd3:
            {
                //OUT
                state->pc += 1;
                cycles += 10;
                break;
            }
            case 0xd5:
            {
                //PUSH
                state->memory[state->sp - 1] = state->d;
                state->memory[state->sp - 2] = state->e;
                state->sp = state->sp - 2;
                cycles += 11;
                break;
            }
            case 0xe1:
            {
                //POP
                state->l = state->memory[state->sp];
                state->h = state->memory[state->sp + 1];
                state->sp = state->sp + 2;
                cycles += 10;
                break;
            }
            case 0xe5:
            {
                //PUSH
                state->memory[state->sp - 1] = state->h;
                state->memory[state->sp - 2] = state->l;
                state->sp = state->sp - 2;
                cycles += 11;
                break;
            }
            case 0xe6:
            {
                //ANI
                state->a = state->a & opcode[1];
                CheckFlags(state->a, state);
                state->pc += 1;
                cycles += 7;
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
                cycles += 4;
                break;
            }
            case 0xf1:
            {
                //POP
                state->a = state->memory[state->sp + 1];
                uint8_t psw = state->memory[state->sp];
                state->cc.z  = (0x01 == (psw & 0x01));
                state->cc.s  = (0x02 == (psw & 0x02));
                state->cc.p  = (0x04 == (psw & 0x04));
                state->cc.cy = (0x05 == (psw & 0x08));
                state->cc.ac = (0x10 == (psw & 0x10));
                state->sp += 2;
                cycles += 10;
                break;
            }
            case 0xf5:
            {
                //PUSH
                state->memory[state->sp - 1] = state->a;
                uint8_t psw = (state->cc.z |
                                state->cc.s << 1 |
                                state->cc.p << 2 |
                                state->cc.cy << 3 |
                                state->cc.ac << 4 );
                state->memory[state->sp - 2] = psw;
                state->sp = state->sp - 2;
                cycles += 11;
                break;
            }
            case 0xfb:
            {
                //EI
                state->cc.interrupt_enabled = 1;
                cycles += 4;
                break;
            }
            case 0xfe:
            {
                //CPI
                CheckFlags((state->a - opcode[1]), state);
                state->pc += 1;
                cycles += 7;
                break;
            }
            case 0xff: UnimplementedInstruction(state); break;
        }
        printf("Carry=%d, Parity=%d, Sign=%d, Zero=%d, Cycles=%d\n", state->cc.cy, state->cc.p,
            state->cc.s, state->cc.z, cycles);
        printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n\n",
            state->a, state->b, state->c, state->d,
            state->e, state->h, state->l, state->sp);
    }
    
}