#ifndef _cpu_h
#define _cpu_h
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
    uint8_t port1;
    uint8_t port2;
    uint16_t shift0, shift1;
    uint8_t shift_offset;
    uint16_t outport3;
    uint16_t outport5;
} State8080;

void UnimplementedInstruction(State8080* state);
void EI();
int Emulate8080Op(State8080* state);

#endif