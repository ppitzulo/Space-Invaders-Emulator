#include <stdint.h>


typedef struct ConditionCodes {
    uint8_t    z:1;
    uint8_t    s:1;
    uint8_t    p:1;
    uint8_t    cy:1;
    uint8_t    ac:1;
    uint8_t    interrupt_enabled:1;
    uint8_t    pad:3;
} ConditionCodes;

typedef struct State8080 {
    uint8_t    a;
    uint8_t    b;
    uint8_t    c;
    uint8_t    d;
    uint8_t    e;
    uint8_t    h;
    uint8_t    l;
    uint16_t    sp;
    uint16_t    pc;
    uint8_t     *memory;
    struct      ConditionCodes      cc;
    uint8_t     int_enable;
} State8080;


void CheckFlags(uint16_t result)
{
    // This might not work.
    if (result & 0xff)
    {
        state.cc.z = 0;
    }
    else
    {
        state.cc.z = 1;
    }

    if (result & 0x80)
    {
        state.cc.s = 1;
    }
    else
    {
        state.cc.s = 0;
    }

    if (result > 0xff)
    {
        state.cc.cy = 1;
    }
    else
    {
        state.cc.cy = 0;
    }

    // Calculate parity
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
        state.cc.p = 1;
    }
    else
    {
        state.cc.p = 0;
    }
}


void UnimplementedInstruction(State8080* state)
{
    //pc will have advanced one, so undo that
    printf("Error: Unimplemented instruction\n");
    printf("%d", state->memory[state->pc]);
    exit(1);
}


void LXI(uint8_t *x, uint8_t *y, unsigned char *opcode)
{
    // WORKS
    *y = opcode[1];
    *x = opcode[2];
    state.pc += 2;
}


void INX(uint8_t *a, uint8_t *b)
{
    // WORKS
    uint16_t pair = (*a<<8) | (*b);
    pair++;
    *a = pair >> 8;
    *b = pair;
}


void INR(uint8_t *x)
{
    // WORKS
    (*x)++;
    CheckFlags(*x);
}


void DCR(uint8_t *x)
{
    // WORKS
    (*x)--;
    CheckFlags(*x);
}


void MVI(uint8_t *x, unsigned char *opcode)
{
    *x = opcode[1];
    state.pc += 1;
}


void DAD(uint8_t *h, uint8_t *l, uint8_t *x, uint8_t *y)
{
    uint16_t pair2 = (*x + *y);
    uint16_t result = (*h + *l) + pair2;
    *h = result>>8;
    *l = result<<8;
    CheckFlags(result);
}


void STAX(uint8_t *a, uint8_t *b)
{
    uint16_t memory_address = (*a<<8) | *b;
    state.memory[memory_address] = state.a;
}


void LDAX(uint8_t *a, uint8_t *b)
{
    // WORKS
    uint16_t memory_address = (*a<<8) | *b;
    state.a = state.memory[memory_address];
}


void STA(uint8_t low, uint8_t high)
{
    uint16_t memory_address = (high<<8) | low;
    state.memory[memory_address] = state.a;
    state.pc += 2;
}


void LDA(uint8_t low, uint8_t high)
{
    uint16_t memory_address = (high<<8) | low;
    state.a = state.memory[memory_address];
    state.pc += 2;
}


void MOV(uint8_t *dest, uint8_t src)
{
    *dest = src;
}


void CPI(uint8_t data)
{
    if(state.a - data)
    {
        state.cc.z = 0;
    }
    else
    {
        state.cc.z = 1;
    }
    CheckFlags(state.a - data);
    state.pc += 2;
}


void ANA(uint8_t reg)
{
    state.a = state.a & reg;
    CheckFlags(state.a);
}


void XRA(uint8_t reg)
{
    state.a = state.a ^ reg;
    CheckFlags(state.a);
}


void POP(uint8_t *high, uint8_t *low)
{
    *low = state.memory[state.sp];
    *high = state.memory[state.sp++];
    state.sp = state.sp + 2;
}


void JNZ(uint16_t memory_address)
{
    // WORKS
    if (state.cc.z)
    {
        state.pc += 2;
    }
    else
    {
        state.pc = memory_address;
    }
}


void JMP(uint16_t memory_address)
{
    // WORKS
    state.pc = memory_address;
}


void PUSH(uint8_t high, uint8_t low)
{
    state.memory[state.sp - 1] = high;
    state.memory[state.sp - 2] = low;
    state.sp = state.sp - 2;
}


void ADI(uint8_t data)
{
    state.a += data;
    CheckFlags(state.a);
    state.pc += 2;
}


void RET()
{
    state.pc = state.memory[state.sp] | (state.memory[state.sp+1] << 8);    
    state.sp += 2;    
}


void CALL(uint16_t address)
{
    // WORKS
    state.memory[state.pc - 1] = state.pc>>8;
    state.memory[state.pc - 2] = state.pc<<8;
    state.sp = state.sp - 2;
    state.pc = address;
}


void OUT()
{
    state.pc += 2;
}


void ANI(uint8_t data)
{
    state.a = state.a & data;
    CheckFlags(state.a);
    state.pc += 2;
}


void XCHG()
{
    uint8_t buffer;
    buffer = state.h;
    state.h = state.d;
    state.d = buffer;
    buffer = state.l;
    state.l = state.e;
    state.e = buffer;
}


void EI()
{
    state.cc.interrupt_enabled = 1;
}


void RRC()
{
    state.a = state.a>>1;
    state.pc++;
    CheckFlags(state.a);
}


int Emulate8080Op(State8080* state)
{
    unsigned char *opcode = &state->memory[state->pc];
    Disassemble8080Op(state->memory, state->pc);
    
    state->pc++;

    switch(*opcode)
    {
        case 0x00: break;
        case 0x01:
        {
            //LXI
            state->c = opcode[1];
            state->b = opcode[2];
            state.pc += 2;
            break;
        }
        case 0x02:
        {
            //STAX
            uint16_t memory_address = (state->b<<8) | state->c;
            state.memory[memory_address] = state.a;
            break;
        }
        case 0x03:
        {
            //INX
            uint16_t pair = (state->b<<8) | state->c;
            pair++;
            state->b = pair >> 8;
            state->c = pair;
            break;
        }
        case 0x04:
        {
            //INR
            state->b++;
            CheckFlags(state->b);
            break;
        }
        case 0x05:
        {
            //DCR
            state->b--;
            CheckFlags(state->b);
            break;
        }
        case 0x06:
        {
            //MVI
            state->b = opcode[1];
            state.pc += 1;
            break;
        }
        case 0x09:
        {
            //DAD
            uint16_t BC = (state->b<<8) | state->c;
            uint16_t HL = (state->h<<8) | state>l;
            uint16_t result = HL + BC;
            state->h = result>>8;
            state->l = result<<8;
            CheckFlags(result);
            break;
        }
        case 0x0d:
		{
            //DCR
            state->c--;
            CheckFlags(state->c);
            break;
        }
        case 0x0e:
        {
            //MVI
            state->c = opcode[1];
            state.pc += 1;
            break;
        }
        case 0x0f:
        {
            state.a = state.a>>1;
            state.pc++;
            CheckFlags(state.a);
            break;
        }
        case 0x11:
        {
            //LXI
            state->e = opcode[1];
            state->d = opcode[2];
            state.pc += 2;
            break;
        }
        case 0x13:
        {
            //INX
            uint16_t pair = (state->d<<8) | state->e;
            pair++;
            state->d = pair >> 8;
            state->e = pair;
            break;
        }
        case 0x19:
        {
            //DAD
            uint16_t DE = (state->d<<8) | state->e;
            uint16_t HL = (state->h<<8) | state>l;
            uint16_t result = HL + DE;
            state->h = result>>8;
            state->l = result<<8;
            CheckFlags(result);
            DAD(&state->h, &state->l, &state->d, &state->e);
            break;
        }
		case 0x1a:
        {
            //LDAX
            uint16_t memory_address = (state->d<<8) | state->e;
            state.a = state.memory[memory_address];
		    break;
        }
	    case 0x21:
	    {
            //LXI
            state->l = opcode[1];
            state->h = opcode[2];
            state.pc += 2;
		    break;
        }
        case 0x23:
        {
            //INX
            uint16_t pair = (state->h<<8) | state->l;
            pair++;
            *state->h = pair >> 8;
            *state->l = pair;
            break;
        }
        case 0x26:
        {
            state->h = opcode[1];
            state.pc += 1;
            break;
        }
        case 0x29:
        {
            uint16_t HL = (state->h<<8) | state>l;
            uint16_t result = (HL * 2);
            state->h = result>>8;
            state->l = result<<8;
            CheckFlags(result);
            DAD(&state->h, &state->l, &state->h, &state->l);
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
            state.pc += 2;
            break;
        }
        case 0x32:
        {
            //STA
            uint16_t memory_address = (opcode[2]<<8) | opcode[1];
            state.memory[memory_address] = state.a;
            state.pc += 2;
            break;
        }
        case 0x36:
        {
            //MVI
            uint16_t HL = (state->h<<8) | state->l;
            state.memory[HL] = opcode[1];
            state.pc += 1;
            break;
        }
        case 0x3a:
        {
            //LDA
            uint16_t memory_address = (opcode[2]<<8) | opcode[1];
            state.a = state.memory[memory_address];
            state.pc += 2;
            break;
        }
        case 0x3e:
        {
            //MVI
            state->a = opcode[1];
            state.pc += 1;
            break;
        }
        case 0x56:
        {
            //MOV
            uint16_t HL = (state->h<<8) | state->l;
            state->d = state->memory[HL];
            break;
        }
        case 0x5e:
        {
            //MOV
            uint16_t HL = (state->h<<8) | state->l;
            state->e = state->memory[HL];
            break;
        }
        case 0x66:
        {
            //MOV
            uint16_t HL = (state->h<<8) | state->l;
            state->h = state->memory[HL];
            break;
        }
        case 0x6f:
        {
            //MOV
            state-l = state->a;
            break;
        }
        case 0x77:
        {
            //MOV
            uint16_t HL = (state->h<<8) | state->l;
            state->memory[HL] = state->a;
            break;
        }
        case 0x7a:
        {
            *dest = src;
            MOV(&state->a, state->d);
            break;
        }
        case 0x7b:
        {
            //MOV
            state->a = state->e;
            break;
        }
        case 0x7c:
        {
            //MOV
            state->a = state->h;
            break;
        }
        case 0x7e:
        {
            //MOV 
            uint16_t HL = (state->h<<8) | state->l;
            state->a = state->memory[HL];
            break;
        }
        case 0xa7:
        {
            //ANA
            state->a = state->a & state->a;
            CheckFlags(state->a);
            break;
        }
        case 0xaf:
        {
            //XRA
            state.a = state->a ^ state->a;
            CheckFlags(state->a);
            break;
        }
        case 0xc1:
        {
            //POP
            state->c = state.memory[state->sp];
            state->b = state.memory[state->sp++];
            state.sp = state.sp + 2;
            break;
        }
        case 0xc2:
        {
            //JNZ
            uint16_t memory_address = (opcode[2] << 8) | opcode[1];
            if (state.cc.z)
            {
                state.pc += 2;
            }
            else
            {
                state.pc = memory_address;
            }
            break;
        }
        case 0xc3:
        {
            //JMP
            state->pc = (opcode[2] << 8) | opcode[1];
            break;
        }
        case 0xc5:
        {
            //PUSH
            state->memory[state.sp - 1] = state->b;
            state->memory[state.sp - 2] = state->c;
            state->sp = state.sp - 2;
            break;
        }
        case 0xc6:
        {
            //ADI
            state->a += opcode[1];
            CheckFlags(state->a);
            state->pc += 2;
            break;
        }
        case 0xc9:
        {
            //RET
            state.pc = state.memory[state.sp] | (state.memory[state.sp+1] << 8);    
            state.sp += 2;  
            break;
        }
        case 0xcd:
        {
            //CALL
            state.memory[state.pc - 1] = state.pc>>8;
            state.memory[state.pc - 2] = state.pc<<8;
            state.sp = state.sp - 2;
            state.pc = (opcode[2]<<8) | opcode[1];
            break;
        }
        case 0xd1:
        {
            //POP
            state->e = state.memory[state->sp];
            state->d = state.memory[state->sp++];
            state.sp = state.sp + 2;
            break;
        }
        case 0xd3:
        {
            //OUT
            state.pc += 2;
            break;
        }
        case 0xd5:
        {
            //PUSH
            state.memory[state.sp - 1] = state->d;
            state.memory[state.sp - 2] = state->e;
            state.sp = state.sp - 2;
            break;
        }
        case 0xe1:
        {
            //POP
            state->l = state.memory[state.sp];
            state->h = state.memory[state.sp++];
            state.sp = state.sp + 2;
            break;
        }
        case 0xe5:
        {
            //PUSH
            state.memory[state.sp - 1] = state->h;
            state.memory[state.sp - 2] = state->l;
            state.sp = state.sp - 2;
            break;
        }
        case 0xe6:
        {
            state.a = state.a & opcode[1];
            CheckFlags(state.a);
            state.pc += 2;
            break;
        }
        case 0xeb:
        {
            //XCHG
            uint8_t buffer;
            buffer = state.h;
            state.h = state.d;
            state.d = buffer;
            buffer = state.l;
            state.l = state.e;
            state.e = buffer;
            break;
        }
        case 0xf1:
        {
            state->a = state->memory[state->sp+1];
            uint8_t psw = state->memory[state->sp];
            state->cc.z  = (0x01 == (psw & 0x01));
            state->cc.s  = (0x02 == (psw & 0x02));
            state->cc.p  = (0x04 == (psw & 0x04));
            state->cc.cy = (0x05 == (psw & 0x08));
            state->cc.ac = (0x10 == (psw & 0x10));
            state->sp += 2;
        }
        case 0xf5:
        {

        }
        case 0xfb:
        {
            state->memory[state->sp-1] = state->a;
            uint8_t psw = (state->cc.z |
                            state->cc.s << 1 |
                            state->cc.p << 2 |
                            state->cc.cy << 3 |
                            state->cc.ac << 4 );
            state->memory[state->sp-2] = psw;
            state->sp = state->sp - 2;
            break;
        }
        case 0xfe:
        {
            if(state.a - opcode[1])
            {
                state.cc.z = 0;
            }
            else
            {
                state.cc.z = 1;
            }
            CheckFlags(state.a - opcode[1]);
            state.pc += 2;
            break;
        }
        case 0xff: UnimplementedInstruction(state); break;
    }
    printf("\tCarry=%d, Parity=%d, Sign=%d, Zero=%d\n", state->cc.cy, state->cc.p,
           state->cc.s, state->cc.z);
    printf("\tA $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n\n",
           state->a, state->b, state->c, state->d,
           state->e, state->h, state->l, state->sp);
    
}