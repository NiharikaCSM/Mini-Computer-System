#ifndef PROCESSOR_H
#define PROCESSOR_H

extern int Register[256];
extern int VRegister[32][8];

extern int PC;
extern int opcode, dest, src1, src2;
extern int end_of_simulation;

void printVectorRegisters(void);
void reset(void);
void fetch(void);
void decode(void);
void execute(void);

#endif
