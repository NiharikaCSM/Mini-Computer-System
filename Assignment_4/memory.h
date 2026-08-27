#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

extern char Instruction[NP][INSTR_MEM_SIZE];
extern char Data[NP][DATA_MEM_SIZE];

void initialize(int proc_id, const char *programFile, const char *dataFile);
void finalize(int proc_id, const char *dataFile);

#endif
