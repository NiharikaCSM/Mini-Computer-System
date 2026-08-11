#include <stdio.h>
#include "processor.h"
#include "memory.h" //needed to access instruction and data array

int Register[256];
int PC;
int opcode, dest, src1, src2; 
int end_of_simulation = 0;

void reset(void) {
    for (int i = 0; i < 256; i++) {
        Register[i] = 0;
    }
    PC = 0;
    end_of_simulation = 0;
}

void fetch(void) {
    //values fetched from current instruction
    opcode = Instruction[PC];
    dest   = Instruction[PC+1];
    src1   = Instruction[PC+2];
    src2   = Instruction[PC+3];
    PC += 4;
    printf("%d %d %d %d\n", opcode, dest, src1, src2); 

}

void decode(void) {}

void execute(void) {
    switch (opcode) {
        case 1: //Add
            Register[dest] = Register[src1] + Register[src2];
            break;
        case 2: //Subtract 
            Register[dest] = Register[src1] - Register[src2];
            break;
        case 3: //Multiply
            Register[dest] = Register[src1] * Register[src2];
            break;
        case 4: //Divide
            if (Register[src2] == 0) 
                printf("Error : Cannot divide by zero\n");
            else 
                Register[dest] = Register[src1] / Register[src2];
            break;
            
        case 5: //Memory read
            Register[dest] = Data[src1];
            break;
        case 6: //Memory write
            Data[src1] = Register[dest];
            break;
        case 7: //Data movement
            Register[dest] = src1;
            break;
        case 0: //Halt
            end_of_simulation = 1;
            break;
        default:
            fprintf(stderr, "Unknown opcode %d at PC=%d\n", opcode, PC - 4);
            end_of_simulation = 1;
            break;
    }
}
