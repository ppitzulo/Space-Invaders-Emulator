#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "disassembler.h"
#include "cpu.h"

#define DEBUG 0

static int parity(int x, int size)
{
    uint8_t nb_one_bits = 0;

    for (int i = 0; i < size; i++)
    {
        nb_one_bits += ((x >> i) & 1);
    }

    return (nb_one_bits & 1) == 0;
}

void flagsZSP(uint16_t value, State8080 *state)
{
    // Handle status flags of every other operation
    state->cc.z = (value == 0);
    state->cc.s = (0x80 == (value & 0x80));
    state->cc.p = parity(value, 8);
}

void flagsA(uint16_t value, State8080 *state)
{
    // Handle the status flags for arithmetic operations
    flagsZSP(value, state);
    state->cc.cy = (value >= 0xff);
}

void flagsL(State8080 *state)
{
    // Handle the status flags for logical operations
    flagsZSP(state->a, state);
    state->cc.cy = 0;
}

void CheckFlags(uint16_t result, State8080 *state)
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

int call(State8080 *state, unsigned char *opcode, int *cycles)
{
    // CALL
#if DEBUG
    if (state->pc == 5)
    {
        if (state->c == 9)
        {
            uint16_t offset = (state->d << 8) | (state->e);
            char *str = &state->memory[offset + 3]; // skip the prefix bytes
            while (*str != '$')
                printf("%c", *str++);
            printf("\n");
            return 0;
        }
        else if (state->c == 2)
        {
            printf("%c", state->e);
            return 0;
        }
    }
    else if (0 == ((opcode[2] << 8) | opcode[1]))
    {
        exit(0);
    }
    else
#endif
        uint16_t ret = state->pc + 2;
    state->sp = state->sp - 2;
    state->memory[state->sp + 1] = (ret >> 8) & 0xff;
    state->memory[state->sp] = (ret & 0xff);
    state->pc = (opcode[2] << 8) | opcode[1];
    return 1;

    cycles += 17;
}

static const uint8_t OPCODES_CYCLES[256] = {
    //  0  1   2   3   4   5   6   7   8  9   A   B   C   D   E  F
    4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4,           // 0
    4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4,           // 1
    4, 10, 16, 5, 5, 5, 7, 4, 4, 10, 16, 5, 5, 5, 7, 4,         // 2
    4, 10, 13, 5, 10, 10, 10, 4, 4, 10, 13, 5, 5, 5, 7, 4,      // 3
    5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,             // 4
    5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,             // 5
    5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,             // 6
    7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 7, 5,             // 7
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,             // 8
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,             // 9
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,             // A
    4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,             // B
    5, 10, 10, 10, 11, 11, 7, 11, 5, 10, 10, 10, 11, 17, 7, 11, // C
    5, 10, 10, 10, 11, 11, 7, 11, 5, 10, 10, 10, 11, 17, 7, 11, // D
    5, 10, 10, 18, 11, 11, 7, 11, 5, 5, 10, 4, 11, 17, 7, 11,   // E
    5, 10, 10, 4, 11, 11, 7, 11, 5, 5, 10, 4, 11, 17, 7, 11     // F
};

void UnimplementedInstruction(State8080 *state)
{
    // pc will have advanced one, so undo that
    printf("Error: Unimplemented instruction\n");
    printf("%d", state->memory[state->pc]);
    exit(1);
}

uint16_t machineIN(State8080 state, uint8_t port)
{
    uint8_t value = 0xff;

    switch (port)
    {
        printf("%d\n", port);
    case 0:
        return 0xf;
    case 1:
        value = state.port1;
        break;
    case 2:
        value = state.port2;
        break;
    case 3:
    {
        uint16_t v = (state.shift1 << 8) | state.shift0;
        value = ((v >> (8 - state.shift_offset)) & 0xff);
        break;
    }
    }
    return value;
}

int Emulate8080Op(State8080 *state)
{
    int cycles = 0;
    unsigned char *opcode = &state->memory[state->pc];
    state->pc++;
    switch (*opcode)
    {
        case 0x00:
        {
            cycles += 4;
            break;
        }
        case 0x01:
        {
            // LXI
            state->c = opcode[1];
            state->b = opcode[2];
            state->pc += 2;
            cycles += 10;
            break;
        }
        case 0x02:
        {
            // STAX
            uint16_t memory_address = (state->b << 8) | state->c;
            state->memory[memory_address] = state->a;
            cycles += 7;
            break;
        }
        case 0x03:
        {
            // INX
            //  ***
            uint16_t pair = (state->b << 8) | state->c;
            pair++;
            state->b = pair >> 8;
            state->c = pair & 0xff;
            cycles += 5;
            break;
        }
        case 0x04:
        {
            // INR
            flagsZSP(state->b + 1, state);
            state->b++;
            state->cc.ac = ((state->b & 0x0f) + (1 & 0x0f) > 0xf);
            cycles += 5;
            break;
        }
        case 0x05:
        {
            // DCR
            state->b--;
            state->cc.ac = !((state->b & 0xf) == 0xf);
            flagsZSP(state->b, state);
            cycles += 5;
            break;
        }
        case 0x06:
        {
            // MVI
            state->b = opcode[1];
            state->pc += 1;
            cycles += 7;
            break;
        }
        case 0x07:
        {
            // RLC
            uint8_t tmp = state->a;
            state->a = ((tmp & 0x80) >> 7) | (tmp << 1);
            state->cc.cy = (0x80 == (tmp & 0x80));
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
            // DAD
            uint16_t BC = (state->b << 8) | state->c;
            uint32_t HL = (state->h << 8) | state->l;
            HL += BC;
            state->h = HL >> 8;
            state->l = HL & 0xff;
            state->cc.cy = (HL >> 16) & 1;
            cycles += 10;
            break;
        }
        case 0x0a:
        {
            // LDAX B
            uint16_t memory_address = (state->b << 8) | state->c;
            state->a = state->memory[memory_address];
            cycles += 7;
            break;
        }
        case 0x0b:
        {
            // DCX
            uint16_t bc = (state->b << 8) | state->c;
            bc--;
            state->b = bc >> 8;
            state->c = bc & 0xff;
            cycles += 5;
            break;
        }
        case 0x0c:
        {
            // INR C
            state->cc.ac = ((state->c & 0x0f) + (1 & 0x0f) > 0xf);
            state->c++;
            flagsZSP(state->c, state);
            cycles += 5;
            break;
        }
        case 0x0d:
        {
            // DCR
            state->c--;
            state->cc.ac = !((state->c & 0xf) == 0xf);
            flagsZSP(state->c, state);
            cycles += 5;
            break;
        }
        case 0x0e:
        {
            // MVI
            state->c = opcode[1];
            state->pc += 1;
            cycles += 7;
            break;
        }
        case 0x0f:
        {
            // RRC
            uint8_t x = state->a;
            state->a = ((x & 1) << 7) | (x >> 1);
            state->cc.cy = (1 == (x & 1));
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
            // LXI
            state->e = opcode[1];
            state->d = opcode[2];
            state->pc += 2;
            cycles += 10;
            break;
        }
        case 0x12:
        {
            // STAX D
            uint16_t memory_address = (state->d << 8) | state->e;
            state->memory[memory_address] = state->a;
            cycles += 7;
            break;
        }
        case 0x13:
        {
            // INX
            uint16_t pair = (state->d << 8) | state->e;
            pair++;
            state->d = pair >> 8;
            state->e = pair & 0xff;
            cycles += 5;
            break;
        }
        case 0x14:
        {
            // INR D
            state->cc.ac = ((state->d & 0x0f) + (1 & 0x0f) > 0xf);
            state->d++;
            flagsZSP(state->d, state);
            cycles += 5;
            break;
        }
        case 0x15:
        {
            // DCR D
            state->d--;
            flagsZSP(state->d, state);
            state->cc.ac = !((state->d & 0xf) == 0xf);
            cycles += 5;
            break;
        }
        case 0x16:
        {
            // MVI D,D8
            state->d = opcode[1];
            state->pc++;
            cycles += 7;
            break;
        }
        case 0x17:
        {
            // RAL
            uint8_t val = state->a;
            state->a = ((state->cc.cy) | (val << 1));
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
            // DAD
            uint32_t DE = (state->d << 8) | state->e;
            uint32_t HL = (state->h << 8) | state->l;
            HL += DE;
            state->h = HL >> 8;
            state->l = HL & 0xff;
            state->cc.cy = (HL >> 16) & 1;
            cycles += 10;
            break;
        }
        case 0x1a:
        {
            // LDAX
            uint16_t memory_address = (state->d << 8) | state->e;
            state->a = state->memory[memory_address];
            cycles += 7;
            break;
        }
        case 0x1b:
        {
            // DCX
            uint16_t de = (state->d << 8) | state->e;
            de--;
            state->d = de >> 8;
            state->e = de & 0xff;
            cycles += 5;
            break;
        }
        case 0x1c:
        {
            state->cc.ac = ((state->e & 0x0f) + (1 & 0x0f) > 0xf);
            state->e++;
            flagsZSP(state->e, state);
            cycles += 5;
            break;
        }
        case 0x1d:
        {
            state->e--;
            flagsZSP(state->e, state);
            state->cc.ac = !((state->e & 0xf) == 0xf);
            cycles += 5;
            break;
        }
        case 0x1e:
        {
            // MVI E,D8
            state->e = opcode[1];
            state->pc++;
            cycles += 7;
            break;
        }
        case 0x1f:
        {
            // RAR
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
            // LXI
            state->l = opcode[1];
            state->h = opcode[2];
            state->pc += 2;
            cycles += 10;
            break;
        }
        case 0x22:
        {
            uint16_t memory_address = (opcode[2] << 8) | opcode[1];
            state->memory[memory_address] = state->l;
            state->memory[memory_address + 1] = state->h;
            state->pc += 2;
            cycles += 16;
            break;
        }
        case 0x23:
        {
            // INX
            uint16_t pair = (state->h << 8) | state->l;
            pair++;
            state->h = pair >> 8;
            state->l = pair & 0xff;
            cycles += 5;
            break;
        }
        case 0x24:
        {
            state->cc.ac = ((state->h & 0x0f) + (1 & 0x0f) > 0xf);
            state->h++;
            flagsZSP(state->h, state);
            cycles = 5;
            break;
        }
        case 0x25:
        {
            state->h--;
            flagsZSP(state->h, state);
            state->cc.ac = !((state->h & 0xf) == 0xf);
            cycles = 5;
            break;
        }
        case 0x26:
        {
            // MVI
            state->h = opcode[1];
            state->pc += 1;
            cycles += 7;
            break;
        }
        case 0x27:
        {
            // DAA
            uint16_t result = state->a;
            int cy = state->cc.cy;
            printf("%d\n", result);
            if ((state->a & 0xf) > 9 || state->cc.ac)
            {
                result += 6;
            }
            if ((state->a >> 4) > 9 || state->cc.cy)
            {
                result += 0x60;
                cy = 1;
            }
            flagsA(result, state);
            state->cc.cy = cy;
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
            // DAD
            uint32_t HL = (state->h << 8) | state->l;
            HL *= 2;
            state->h = HL >> 8;
            state->l = HL & 0xff;
            state->cc.cy = (HL >> 16) & 1;
            cycles += 10;
            break;
        }
        case 0x2a:
        {
            uint16_t memory_address = (opcode[2] << 8) | opcode[1];
            state->l = state->memory[memory_address];
            state->h = state->memory[memory_address + 1];
            state->pc += 2;
            cycles += 16;
            break;
        }
        case 0x2b:
        {
            // DCX H
            uint16_t hl = (state->h << 8) | state->l;
            hl--;
            state->h = hl >> 8;
            state->l = hl & 0xff;
            cycles += 5;
            break;
        }
        case 0x2c:
        {
            // INR L
            state->cc.ac = ((state->l & 0x0f) + (1 & 0x0f) > 0xf);
            state->l++;
            flagsZSP(state->l, state);
            cycles += 5;
            break;
        }
        case 0x2d:
        {
            // DCR L
            state->l--;
            flagsZSP(state->l, state);
            state->cc.ac = !((state->l & 0xf) == 0xf);
            cycles += 5;
            break;
        }
        case 0x2e:
        {
            // MVI L, D8
            state->l = opcode[1];
            state->pc++;
            cycles += 7;
            break;
        }
        case 0x2f:
        {
            state->a = ~state->a;
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
            // LXI
            state->sp = (opcode[2] << 8) | opcode[1];
            state->pc += 2;
            cycles += 10;
            break;
        }
        case 0x32:
        {
            // STA
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
            // INR M
            uint16_t memory_address = (state->h << 8) | state->l;
            state->cc.ac = ((state->memory[memory_address] & 0x0f) + (1 & 0x0f) > 0xf);
            state->memory[memory_address] = (state->memory[memory_address] + 1);
            flagsZSP(state->memory[memory_address], state);
            cycles += 10;
            break;
        }
        case 0x35:
        {
            // DCR M
            uint16_t memory_address = (state->h << 8) | state->l;
            state->memory[memory_address] = (state->memory[memory_address] - 1);
            flagsZSP(state->memory[memory_address], state);
            state->cc.ac = !((state->memory[memory_address] & 0xf) == 0xf);
            cycles += 10;
            break;
        }
        case 0x36:
        {
            // MVI
            uint16_t HL = (state->h << 8) | state->l;
            state->memory[HL] = opcode[1];
            state->pc += 1;
            cycles += 10;
            break;
        }
        case 0x37:
        {
            state->cc.cy = 1;
            cycles += 4;
            break;
        }
        case 0x38:
        {
            cycles += 4;
            break;
        }
        case 0x39:
        {
            uint32_t HL = (state->h << 8) | state->l;
            HL = HL + state->sp;
            state->h = (HL & 0xff00) >> 8;
            state->l = HL & 0xFF;
            state->cc.cy = (HL >> 16) & 1;
            cycles += 10;
            break;
        }
        case 0x3a:
        {
            // LDA
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
            state->cc.ac = ((state->a & 0x0f) + (1 & 0x0f) > 0xf);
            state->a++;
            flagsZSP(state->a, state);
            cycles += 5;
            break;
        }
        case 0x3d:
        {
            state->a--;
            flagsZSP(state->a, state);
            state->cc.ac = !((state->a & 0xf) == 0xf);
            cycles += 5;
            break;
        }
        case 0x3e:
        {
            // MVI
            state->a = opcode[1];
            state->pc += 1;
            cycles += 7;
            break;
        }
        case 0x3f:
        {
            // CMC
            state->cc.cy = 0;
            cycles += 4;
            break;
        }
        case 0x40:
        {
            state->b = state->b;
            cycles += 5;
            break;
        }
        case 0x41:
        {
            state->b = state->c;
            cycles += 5;
            break;
        }
        case 0x42:
        {
            state->b = state->d;
            cycles += 5;
            break;
        }
        case 0x43:
        {
            state->b = state->e;
            cycles += 5;
            break;
        }
        case 0x44:
        {
            state->b = state->h;
            cycles += 5;
            break;
        }
        case 0x45:
        {
            state->b = state->l;
            cycles += 5;
            break;
        }
        case 0x46:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->b = state->memory[memory_address];
            cycles += 7;
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
            break;
        }
        case 0x49:
        {
            state->c = state->c;
            cycles += 5;
            break;
        }
        case 0x4a:
        {
            state->c = state->d;
            cycles += 5;
            break;
        }
        case 0x4b:
        {
            state->c = state->e;
            cycles += 5;
            break;
        }
        case 0x4c:
        {
            state->c = state->h;
            cycles += 5;
            break;
        }
        case 0x4d:
        {
            state->c = state->l;
            cycles += 5;
            break;
        }
        case 0x4e:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->c = state->memory[memory_address];
            cycles += 7;
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
            break;
        }
        case 0x51:
        {
            state->d = state->c;
            cycles += 5;
            break;
        }
        case 0x52:
        {
            state->d = state->d;
            cycles += 5;
            break;
        }
        case 0x53:
        {
            state->d = state->e;
            cycles += 5;
            break;
        }
        case 0x54:
        {
            state->d = state->h;
            cycles += 5;
            break;
        }
        case 0x55:
        {
            state->d = state->l;
            cycles += 5;
            break;
        }
        case 0x56:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->d = state->memory[memory_address];
            cycles += 7;
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
            break;
        }
        case 0x59:
        {
            state->e = state->c;
            cycles += 5;
            break;
        }
        case 0x5a:
        {
            state->e = state->d;
            cycles += 5;
            break;
        }
        case 0x5b:
        {
            state->e = state->e;
            cycles += 5;
            break;
        }
        case 0x5c:
        {
            state->e = state->h;
            cycles += 5;
            break;
        }
        case 0x5d:
        {
            state->e = state->l;
            cycles += 5;
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
            cycles += 5;
            break;
        }
        case 0x60:
        {
            state->h = state->b;
            cycles += 5;
            break;
        }
        case 0x61:
        {
            state->h = state->c;
            cycles += 5;
            break;
        }
        case 0x62:
        {
            state->h = state->d;
            cycles += 5;
            break;
        }
        case 0x63:
        {
            state->h = state->e;
            cycles += 5;
            break;
        }
        case 0x64:
        {
            state->h = state->h;
            cycles += 5;
            break;
        }
        case 0x65:
        {
            state->h = state->l;
            cycles += 5;
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
            break;
        }
        case 0x69:
        {
            state->l = state->c;
            cycles += 5;
            break;
        }
        case 0x6a:
        {
            state->l = state->d;
            cycles += 5;
            break;
        }
        case 0x6b:
        {
            state->l = state->e;
            cycles += 5;
            break;
        }
        case 0x6c:
        {
            state->l = state->h;
            cycles += 5;
            break;
        }
        case 0x6d:
        {
            state->l = state->l;
            cycles += 5;
            break;
        }
        case 0x6e:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->l = state->memory[memory_address];
            cycles += 7;
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
            break;
        }
        case 0x71:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->memory[memory_address] = state->c;
            cycles += 7;
            break;
        }
        case 0x72:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->memory[memory_address] = state->d;
            cycles += 7;
            break;
        }
        case 0x73:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->memory[memory_address] = state->e;
            cycles += 7;
            break;
        }
        case 0x74:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->memory[memory_address] = state->h;
            cycles += 7;
            break;
        }
        case 0x75:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->memory[memory_address] = state->l;
            cycles += 7;
            break;
        }
        case 0x76:
        {
            cycles += 7;
            break;
        }
        case 0x77:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->memory[memory_address] = state->a;
            cycles += 7;
            break;
        }
        case 0x78:
        {
            state->a = state->b;
            cycles += 5;
            break;
        }
        case 0x79:
        {
            state->a = state->c;
            cycles += 5;
            break;
        }
        case 0x7a:
        {
            state->a = state->d;
            cycles += 5;
            break;
        }
        case 0x7b:
        {
            state->a = state->e;
            cycles += 5;
            break;
        }
        case 0x7c:
        {
            state->a = state->h;
            cycles += 5;
            break;
        }
        case 0x7d:
        {
            state->a = state->l;
            cycles += 5;
            break;
        }
        case 0x7e:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->a = state->memory[memory_address];
            cycles += 7;
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
            flagsA(state->a + state->b, state);
            state->a = state->a + state->b;
            cycles += 4;
            break;
        }
        case 0x81:
        {
            state->cc.ac = (((state->a & 0xf) + (state->c & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->c;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x82:
        {
            state->cc.ac = (((state->a & 0xf) + (state->d & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->d;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x83:
        {
            state->cc.ac = (((state->a & 0xf) + (state->e & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->e;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x84:
        {
            state->cc.ac = (((state->a & 0xf) + (state->h & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->h;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x85:
        {
            state->cc.ac = (((state->a & 0xf) + (state->l & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->l;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x86:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->cc.ac = (((state->a & 0xf) + (state->memory[memory_address] & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->memory[memory_address];
            flagsA(state->a + state->memory[memory_address], state);
            state->a = result;
            cycles += 7;
            break;
        }
        case 0x87:
        {
            state->cc.ac = (((state->a & 0xf) + (state->a & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->a;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x88:
        {
            state->cc.ac = (((state->a & 0xf) + ((state->b + state->cc.cy) & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->b + state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x89:
        {
            state->cc.ac = (((state->a & 0xf) + ((state->c + state->cc.cy) & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->c + state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x8a:
        {
            state->cc.ac = (((state->a & 0xf) + ((state->d + state->cc.cy) & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->d + state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x8b:
        {
            state->cc.ac = (((state->a & 0xf) + ((state->e + state->cc.cy) & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->e + state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x8c:
        {
            state->cc.ac = (((state->a & 0xf) + ((state->h + state->cc.cy) & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->h + state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x8d:
        {
            state->cc.ac = (((state->a & 0xf) + ((state->l + state->cc.cy) & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->l + state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x8e:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->cc.ac = (((state->a & 0xf) + ((state->memory[memory_address] + state->cc.cy) & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->memory[memory_address] + state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 7;
            break;
        }
        case 0x8f:
        {
            state->cc.ac = (((state->a & 0xf) + ((state->a + state->cc.cy) & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a + state->a + state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x90:
        {
            state->cc.ac = ((state->a & 0xf) - (state->b & 0xf)) & 0x10;
            flagsA(state->a - state->b, state);
            state->a = state->a - state->b;
            cycles += 4;
            break;
        }
        case 0x91:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->c) & 0xf) + 1) > 0xf);
            flagsA(state->a - state->c, state);
            state->a = state->a - state->c;
            cycles += 4;
            break;
        }
        case 0x92:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->d) & 0xf) + 1) > 0xf);
            flagsA(state->a - state->d, state);
            state->a = state->a - state->d;
            cycles += 4;
            break;
        }
        case 0x93:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->e) & 0xf) + 1) > 0xf);
            flagsA(state->a - state->e, state);
            state->a = state->a - state->e;
            cycles += 4;
            break;
        }
        case 0x94:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->h) & 0xf) + 1) > 0xf);
            flagsA(state->a - state->h, state);
            state->a = state->a - state->h;
            cycles += 4;
            break;
        }
        case 0x95:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->l) & 0xf) + 1) > 0xf);
            flagsA(state->a - state->l, state);
            state->a = state->a - state->l;
            cycles += 4;
            break;
        }
        case 0x96:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->memory[memory_address]) & 0xf) + 1) > 0xf);
            flagsA(state->a - state->memory[memory_address], state);
            state->a = state->a - state->memory[memory_address];
            cycles += 7;
            break;
        }
        case 0x97:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->a) & 0xf) + 1) > 0xf);
            flagsA(state->a - state->a, state);
            state->a = state->a - state->a;
            cycles += 4;
            break;
        }
        case 0x98:
        {
            state->cc.ac = ((state->a & 0xf) + ((~state->b) & 0xf) + ~state->cc.cy) > 0xf;
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->b) & 0xf) + !state->cc.cy) > 0xf);
            uint16_t result = state->a - state->b - state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x99:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->c) & 0xf) + !state->cc.cy) > 0xf);
            uint16_t result = state->a - state->c - state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x9a:
        {
            state->cc.ac = ((state->a & 0xf) + ((~state->d) & 0xf) + ~state->cc.cy) > 0xf;
            uint16_t result = state->a - state->d - state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x9b:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->e) & 0xf) + !state->cc.cy) > 0xf);
            uint16_t result = state->a - state->e - state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x9c:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->h) & 0xf) + !state->cc.cy) > 0xf);
            uint16_t result = state->a - state->h - state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x9d:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->l) & 0xf) + !state->cc.cy) > 0xf);
            uint16_t result = state->a - state->l - state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0x9e:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->cc.ac = (((state->a & 0xf) + ((state->memory[memory_address] - state->cc.cy) & 0xf)) & 0x10) == 0x10;
            uint16_t result = state->a - state->memory[memory_address] - state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 7;
            break;
        }
        case 0x9f:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->a) & 0xf) + !state->cc.cy) > 0xf);
            uint16_t result = state->a - state->a - state->cc.cy;
            flagsA(result, state);
            state->a = result;
            cycles += 4;
            break;
        }
        case 0xa0:
        {
            state->cc.ac = ((state->a | state->b) & 0x08) != 0;
            state->a = state->a & state->b;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xa1:
        {

            state->cc.ac = ((state->a >> 3) | (state->c >> 3)) & 1;
            state->a = state->a & state->c;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xa2:
        {
            state->cc.ac = ((state->a | state->d) & 0x08) != 0;
            state->a = state->a & state->d;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xa3:
        {
            state->cc.ac = ((state->a | state->e) & 0x08) != 0;
            state->a = state->a & state->e;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xa4:
        {
            state->cc.ac = ((state->a | state->b) & 0x08) != 0;
            state->a = state->a & state->h;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xa5:
        {
            state->cc.ac = ((state->a | state->l) & 0x08) != 0;
            state->a = state->a & state->l;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xa6:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->cc.ac = ((state->a | state->memory[memory_address]) & 0x08) != 0;
            state->a = state->a & state->memory[memory_address];
            flagsL(state);
            cycles += 7;
            break;
        }
        case 0xa7:
        {
            state->cc.ac = ((state->a | state->a) & 0x08) != 0;
            state->a &= state->a;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xa8:
        {
            state->cc.ac = (((state->a & 0xf) ^ (state->b & 0xf)) & 0x10) == 0x10;
            state->a = state->a ^ state->b;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xa9:
        {
            state->cc.ac = (((state->a & 0xf) ^ (state->c & 0xf)) & 0x10) == 0x10;
            state->a = state->a ^ state->c;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xaa:
        {
            state->cc.ac = (((state->a & 0xf) ^ (state->d & 0xf)) & 0x10) == 0x10;
            state->a = state->a ^ state->d;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xab:
        {
            state->cc.ac = (((state->a & 0xf) ^ (state->e & 0xf)) & 0x10) == 0x10;
            state->a = state->a ^ state->e;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xac:
        {
            state->cc.ac = (((state->a & 0xf) ^ (state->h & 0xf)) & 0x10) == 0x10;
            state->a = state->a ^ state->h;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xad:
        {
            state->cc.ac = (((state->a & 0xf) ^ (state->l & 0xf)) & 0x10) == 0x10;
            state->a = state->a ^ state->l;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xae:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->cc.ac = (((state->a & 0xf) ^ (state->memory[memory_address] & 0xf)) & 0x10) == 0x10;
            state->a = state->a ^ state->memory[memory_address];
            flagsL(state);
            cycles += 7;
            break;
        }
        case 0xaf:
        {
            state->cc.ac = (((state->a & 0xf) ^ (state->a & 0xf)) & 0x10) == 0x10;
            state->a ^= state->a;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xb0:
        {
            state->cc.ac = (((state->a & 0xf) | (state->b & 0xf)) & 0x10) == 0x10;
            state->a = state->a | state->b;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xb1:
        {
            state->cc.ac = (((state->a & 0xf) | (state->c & 0xf)) & 0x10) == 0x10;
            state->a = state->a | state->c;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xb2:
        {
            state->cc.ac = (((state->a & 0xf) | (state->d & 0xf)) & 0x10) == 0x10;
            state->a = state->a | state->d;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xb3:
        {
            state->cc.ac = (((state->a & 0xf) | (state->e & 0xf)) & 0x10) == 0x10;
            state->a = state->a | state->e;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xb4:
        {
            state->cc.ac = (((state->a & 0xf) | (state->h & 0xf)) & 0x10) == 0x10;
            state->a = state->a | state->h;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xb5:
        {
            state->cc.ac = (((state->a & 0xf) | (state->l & 0xf)) & 0x10) == 0x10;
            state->a = state->a | state->l;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xb6:
        {
            uint16_t memory_address = (state->h << 8) | state->l;
            state->cc.ac = (((state->a & 0xf) | (state->memory[memory_address] & 0xf)) & 0x10) == 0x10;
            state->a = state->a | state->memory[memory_address];
            flagsL(state);
            cycles += 7;
            break;
        }
        case 0xb7:
        {
            state->cc.ac = (((state->a & 0xf) | (state->a & 0xf)) & 0x10) == 0x10;
            state->a = state->a | state->a;
            flagsL(state);
            cycles += 4;
            break;
        }
        case 0xb8:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->b) & 0xf) + 1) > 0xf);
            uint16_t result = state->a - state->b;
            flagsA(result, state);
            cycles += 4;
            break;
        }
        case 0xb9:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->c) & 0xf) + 1) > 0xf);
            uint16_t result = state->a - state->c;
            flagsA(result, state);
            cycles += 4;
            break;
        }
        case 0xba:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->d) & 0xf) + 1) > 0xf);
            uint16_t result = state->a - state->d;
            flagsA(result, state);
            cycles += 4;
            break;
        }
        case 0xbb:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->e) & 0xf) + 1) > 0xf);
            uint16_t result = state->a - state->e;
            flagsA(result, state);
            cycles += 4;
            break;
        }
        case 0xbc:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->h) & 0xf) + 1) > 0xf);
            uint16_t result = state->a - state->h;
            flagsA(result, state);
            cycles += 4;
            break;
        }
        case 0xbd:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->l) & 0xf) + 1) > 0xf);
            uint16_t result = state->a - state->l;
            flagsA(result, state);
            cycles += 4;
            break;
        }
        case 0xbe:
        {

            uint16_t memory_address = (state->h << 8) | state->l;
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->memory[memory_address]) & 0xf) + 1) > 0xf);
            uint16_t result = state->a - state->memory[memory_address];
            flagsA(result, state);
            cycles += 7;
            break;
        }
        case 0xbf:
        {
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~state->a) & 0xf) + 1) > 0xf);
            uint16_t result = state->a - state->a;
            state->cc.ac = (((state->a & 0xf) + (state->b & 0xf) & 0x10) == 0x10);
            flagsA(result, state);
            cycles += 4;
            break;
        }
        case 0xc0:
        {
            // NOTE There is a good chance that these might not work
            if (state->cc.z == 0)
            {
                state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
                state->sp += 2;
                return 11;
            }
            cycles += 11;
            break;
        }
        case 0xc1:
        {
            // POP
            state->c = state->memory[state->sp];
            state->b = state->memory[state->sp + 1];
            state->sp = state->sp + 2;
            cycles += 10;
            break;
        }
        case 0xc2:
        {
            // JNZ
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
            // JMP
            state->pc = (opcode[2] << 8) | opcode[1];
            cycles += 10;
            break;
        }
        case 0xc4:
        {
            if (state->cc.z == 0)
            {
                call(state, opcode, &cycles);
                return 17;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        case 0xc5:
        {
            // PUSH
            state->sp = state->sp - 2;
            state->memory[state->sp + 1] = state->b;
            state->memory[state->sp] = state->c;
            cycles += 11;
            break;
        }
        case 0xc6:
        {
            // ADI
            uint16_t tmp = state->a + opcode[1];
            flagsZSP(tmp & 0xff, state);
            state->cc.cy = (tmp > 0xff);
            state->cc.ac = ((state->a & 0x0f) + (opcode[1] & 0x0f) > 0xf);
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
        }
        case 0xc8:
        {
            if (state->cc.z)
            {
                state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
                state->sp += 2;
                return 11;
            }
            cycles += 11;
            break;
        }
        case 0xc9:
        {
            // RET
            state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
            state->sp += 2;
            cycles += 10;
            break;
        }
        case 0xca:
        {
            if (state->cc.z)
            {
                state->pc = (opcode[2] << 8) | opcode[1];
                return 10;
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
            if (state->cc.z == 1)
            {
                call(state, opcode, &cycles);
                return 17;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        case 0xcd:
        {
            if (!call(state, opcode, &cycles))
            {
                state->pc += 2;
            }
            else
            {
                return 17;
            }

            break;
        }
        case 0xce:
        {
            uint16_t tmp = state->a + opcode[1] + state->cc.cy;
            flagsZSP(tmp & 0xff, state);
            state->cc.cy = (tmp > 0xff);
            state->cc.ac = ((state->a & 0x0f) + ((opcode[1] + state->cc.cy) & 0x0f) > 0xf);
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
            break;
        }
        case 0xd1:
        {
            // POP
            state->e = state->memory[state->sp];
            state->d = state->memory[state->sp + 1];
            state->sp = state->sp + 2;
            cycles += 10;
            break;
        }
        case 0xd2:
        {
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
            // OUT
            uint8_t port = opcode[1];
            machineOUT(port, state);
            state->pc += 1;
            cycles += 3;

            break;
        }
        case 0xd4:
        {
            if (state->cc.cy == 0)
            {
                call(state, opcode, &cycles);
                return 17;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        case 0xd5:
        {
            // PUSH
            state->sp = state->sp - 2;
            state->memory[state->sp + 1] = state->d;
            state->memory[state->sp] = state->e;
            cycles += 11;
            break;
        }
        case 0xd6:
        {
            uint8_t tmp = state->a - opcode[1];
            flagsZSP(tmp, state);
            state->cc.cy = (state->a < opcode[1]);
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~opcode[1]) & 0xf) + 1) > 0xf);
            state->a = tmp;
            state->pc++;
            cycles += 7;
            break;
        }
        case 0xd7:
        {
            uint16_t ret = state->pc + 2;
            state->memory[state->sp - 1] = (ret >> 8) & 0xff;
            state->memory[state->sp - 2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = 0x10;
            cycles += 11;
            break;
        }
        case 0xd8:
        {
            if (state->cc.cy != 0)
            {
                state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
                state->sp += 2;
                return 11;
            }
            cycles += 11;

            break;
        }
        case 0xd9:
        {
            cycles += 10;
            break;
        }
        case 0xda:
        {
            if (state->cc.cy)
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
        case 0xdb:
        {
            // IN
            uint8_t port = opcode[1];
            state->a = machineIN(*state, port);
            cycles += 3;
            state->pc++;
            break;
        }
        case 0xdc:
        {
            if (state->cc.cy != 0)
            {
                call(state, opcode, &cycles);
                return 17;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        case 0xdd:
        {
            cycles += 17;
            break;
        }
        case 0xde:
        {
            uint16_t tmp = state->a - opcode[1] - state->cc.cy;
            flagsZSP(tmp & 0xff, state);
            state->cc.cy = (tmp > 0xff);
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~opcode[1]) & 0xf) + 1) > 0xf);
            state->a = tmp & 0xff;
            state->pc++;
            cycles += 7;
            break;
        }
        case 0xdf:
        {
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
            break;
        }
        case 0xe1:
        {
            // POP
            state->l = state->memory[state->sp];
            state->h = state->memory[state->sp + 1];
            state->sp = state->sp + 2;
            cycles += 10;
            break;
        }
        case 0xe2:
        {
            // JMP
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
            state->h = state->memory[(state->sp + 1)];
            state->memory[(state->sp + 1)] = buffer;
            cycles += 18;
            break;
        }
        case 0xe4:
        {
            if (state->cc.p == 0)
            {
                call(state, opcode, &cycles);
                return 17;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        case 0xe5:
        {
            // PUSH
            state->sp = state->sp - 2;
            state->memory[state->sp + 1] = state->h;
            state->memory[state->sp] = state->l;
            cycles += 11;
            break;
        }
        case 0xe6:
        {
            state->cc.ac = ((state->a | opcode[1]) & 0x08) != 0;
            state->a &= opcode[1];
            flagsL(state);
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
            // XCHG
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
            if (state->cc.p != 0)
            {
                call(state, opcode, &cycles);
                return 17;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        case 0xed:
        {
            cycles += 17;
            break;
        }
        case 0xee:
        {
            state->a ^= opcode[1];
            state->cc.ac = 0;
            flagsL(state);
            state->pc++;
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
            if (state->cc.s == 0)
            {
                state->pc = (state->memory[state->sp + 1]) << 8 | state->memory[state->sp];
                state->sp += 2;
                return 11;
            }
            cycles += 11;
            break;
        }
        case 0xf1:
        {
            // POP
            state->a = state->memory[state->sp + 1];
            uint8_t psw = state->memory[state->sp];

            state->cc.s = psw >> 7 & 0x1;
            state->cc.z = (psw >> 6) & 0x1;
            state->cc.ac = (psw >> 4) & 0x1;
            state->cc.p = (psw >> 2) & 0x1;
            state->cc.cy = psw & 0x1;
            state->sp += 2;
            cycles += 10;
            break;
        }
        case 0xf2:
        {
            if (state->cc.s == 0)
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
            state->cc.interrupt_enabled = 0;
            cycles += 4;
            break;
        }
        case 0xf4:
        {
            if (state->cc.s == 0)
            {
                call(state, opcode, &cycles);
                return 17;
            }
            else
            {
                state->pc += 2;
            }
            break;
        }
        case 0xf5:
        {
            // PUSH
            state->sp = state->sp - 2;
            state->memory[state->sp + 1] = state->a;
            uint8_t psw = 0;
            psw |= state->cc.s << 7;
            psw |= state->cc.z << 6;
            psw |= state->cc.ac << 4;
            psw |= state->cc.p << 2;
            psw |= 1 << 1;
            psw |= state->cc.cy << 0;
            state->memory[state->sp] = psw;
            cycles += 11;
            break;
        }
        case 0xf6:
        {
            uint8_t tmp = state->a | opcode[1];
            flagsZSP(tmp, state);
            state->cc.cy = 0;
            state->cc.ac = ((state->a & 0x0f) | (opcode[1] & 0x0f) > 0xf);
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
            else
            {
                state->pc += 2;
            }
            cycles += 10;
            break;
        }
        case 0xfb:
        {
            // EI
            state->cc.interrupt_enabled = 1;
            cycles += 4;
            break;
        }
        case 0xfc:
        {
            if (state->cc.s != 0)
            {
                call(state, opcode, &cycles);
                return 17;
            }
            else
            {
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
            // CPI
            uint16_t tmp = state->a - opcode[1];
            flagsZSP(tmp & 0xff, state);
            state->cc.cy = tmp >> 8;
            state->cc.ac = (unsigned)((state->a & 0xf) + (((~opcode[1]) & 0xf) + 1) > 0xf);
            state->pc += 1;
            cycles += 7;
            break;
        }
        case 0xff:
        {
            uint16_t ret = state->pc + 2;
            state->memory[state->sp - 1] = (ret >> 8) & 0xff;
            state->memory[state->sp - 2] = (ret & 0xff);
            state->sp = state->sp - 2;
            state->pc = 0x38;
            cycles += 11;
            break;
        }
    }
    return OPCODES_CYCLES[*opcode];
}