#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "bus.h"

char Instruction[256];
char Data[4096];

void initialize(void) {
    //make the entries in instruction and data array zero
    memset(Instruction, 0, sizeof(Instruction));
    memset(Data, 0, sizeof(Data));

    FILE *program = fopen("program.byte", "rb");
    if (program) {
        fread(Instruction, sizeof(char), 256, program);
        fclose(program);
    } else {
        printf("Program.byte not found, Instruction memory left as 0.\n");
    }

    FILE *data = fopen("data.byte", "rb");
    if (data) {
        fread(Data, sizeof(char), 4096, data);
        fclose(data);
    } else {
        printf("data.byte not found, Data memory left as 0.\n");
    }
}

void finalize(void) {
    FILE *data = fopen("data.byte", "w");
    if (!data) {
        printf("Error: cannot write data.byte\n");
        return;
    }
    fwrite(Data, sizeof(char), 4096, data);
    fclose(data);
}
