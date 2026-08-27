#include <stdio.h>
#include <string.h>
#include "memory.h"

char Instruction[NP][INSTR_MEM_SIZE];
char Data[NP][DATA_MEM_SIZE];

void initialize(int proc_id, const char *programFile, const char *dataFile) {
    
    memset(Instruction[proc_id], 0, sizeof(Instruction[proc_id]));
    memset(Data[proc_id], 0, sizeof(Data[proc_id]));

    FILE *program = fopen(programFile, "rb");
    if (program) {
        fread(Instruction[proc_id], sizeof(char), INSTR_MEM_SIZE, program);
        fclose(program);
    } else {
        printf("%s not found, Instruction memory left as 0 for proc %d.\n",
               programFile, proc_id);
    }

    if (!dataFile) return; 

    FILE *dataFp = fopen(dataFile, "r");
    if (!dataFp) return;

    int addr = 0;
    unsigned int val;
    while (addr < DATA_MEM_SIZE && fscanf(dataFp, "%x", &val) == 1) {
        Data[proc_id][addr++] = (unsigned char)val;
    }
    fclose(dataFp);
}

void finalize(int proc_id, const char *dataFile) {
    if (!dataFile) return;
    FILE *data = fopen(dataFile, "w");
    if (!data) {
        printf("Error: cannot write %s\n", dataFile);
        return;
    }
    fwrite(Data[proc_id], sizeof(char), DATA_MEM_SIZE, data);
    fclose(data);
}
