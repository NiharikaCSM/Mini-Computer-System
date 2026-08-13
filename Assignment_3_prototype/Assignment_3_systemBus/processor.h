#ifndef PROCESSOR_H
#define PROCESSOR_H

extern int Register[256];
extern int PC;
extern int opcode, dest, src1, src2;
extern int end_of_simulation;

/* Condition flags updated by add/subtract operations (Lab 2) */
extern int Z, N, C, V;

void reset(void);
void fetch(void);
void decode(void);
void execute(void);

#endif
