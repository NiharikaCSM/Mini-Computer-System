#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "common.h"

extern int Register[NP][256];
extern int VRegister[NP][32][8];

extern int PC[NP];
extern int end_of_simulation[NP];

extern int Zf[NP], Nf[NP], Cf[NP], Vf[NP];

void resetProcessor(int proc_id);
void fetch(int proc_id);
void decode(int proc_id);
void execute(int proc_id);
void printVectorRegisters(int proc_id);

void closeProcessorLog(void);

#endif
