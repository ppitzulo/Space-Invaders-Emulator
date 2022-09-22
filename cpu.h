#ifndef _cpu_h
#define _cpu_h

typedef struct ConditionCodes;

typedef struct State8080;



void Checkflags(uint16 result);
void UnimplementedInstruction(State8080* state);
void LXI(uint8_t *x, uint8_t *y, usigned char *opcode);\
void INX(uint8_t *a, uint8_t *b);
void INR(uint8_t *x);
void DCR(uint8_t *x);
void MVI(uint8_t, usigned char *opcode);
void DAD(uint8_t *h, uint8_t *l, uint8_t *x, uint8_t *y);
void STAX(uint8_t *a, uint8_t *b);
void LDAX(uint8_t *a, uint8_t *b);
void STA(uint8_t low, uint8_t high);
void LDA(uint8_t low, uint8_t high);
void MOV(uint8_t *dest, uint8_t src);
void CPI(uint8_t data);
void ANA(uint8_t reg);
void XRA(uint8_t reg);
void POP(uint8_t *high, uint8_t *low);
void JNZ(uint16_t memory_address);
void JMP(uint16_t memory_address);
void PUSH(uint8_t high, uint8_t low);
void ADI(uint8_t data);
void RET();
void CALL(uint16_t address);
void OUT();
void ANI(uint8_t data);
void XCHG();
void EI();
void RRC();
int Emulate8080Op(State8080* state);

#endif