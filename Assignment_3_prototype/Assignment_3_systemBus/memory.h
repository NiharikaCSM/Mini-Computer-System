#ifndef MEMORY_H
#define MEMORY_H

extern char Instruction[256];
extern char Data[4096];

void initialize(void);
void finalize(void);

#endif
