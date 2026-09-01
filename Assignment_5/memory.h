#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

//single physical memory
extern char memory[MEMSIZE];
//maps logical memory to physical memory
extern int pageTable[NP][NUM_LOGICAL_PAGES];
 //freeFrames[i] == 0 -> frame i is free, 1 -> allocated.
extern char freeFrames[NUM_PHYSICAL_PAGES];

void memoryInit(void);
int getFreeFrame(void);
int getPhysicalAddress(int proc_id, int isFetch, int address);
void initialize(int proc_id, const char *programFile, const char *dataFile);
void finalize(int proc_id, const char *dataFile);

#endif
