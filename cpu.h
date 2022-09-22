#ifndef _cpu_h
#define _cpu_h

typedef struct ConditionCodes;

typedef struct State8080;



void Checkflags(uint16 result);
void UnimplementedInstruction(State8080* state);
void EI();
int Emulate8080Op(State8080* state);

#endif