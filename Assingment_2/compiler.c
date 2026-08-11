#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "compiler.h"

static int invalid = 0, outOfRange = 0;


static void cleanUpLine(char *line){
    //p holds the starting address of line
    for (char *p = line; *p; p++) {
        *p = (char)tolower(*p);
    }
}

// returns 1 if every character is whitespace
static int isBlankLine(char *line) {
    for (char *p = line; *p; p++) {
        if (!isspace(*p)) {
            return 0;
        }
    }
    return 1;
}

void compile(const char *inputFile, const char *outputFile) {
    FILE *input = fopen(inputFile, "r");
    FILE *output = fopen(outputFile, "wb");
    if (!input || !output) 
        exit(1);

    char programLine[256];
    while (fgets(programLine, sizeof(programLine), input)) {

        if (isBlankLine(programLine)) {
            continue;
        }
        //final allocation variables
        int opcode = 0, dest = 0, src1 = 0, src2 = 0;
        
        //operation read from program file
        char op;

        cleanUpLine(programLine);

        //matching read
        if (sscanf(programLine, " read x%d, %d", &dest, &src1) == 2) {
            opcode = 5; src2 = 0;
        }
        //matching write
        else if (sscanf(programLine, " write x%d, %d", &dest, &src1) == 2) {
                opcode = 6; src2 = 0;
        }
        //matching arithmatic operations
        else if (sscanf(programLine, " x%d = x%d %c x%d", &dest, &src1, &op, &src2) == 4) {
            if      (op == '+') opcode = 1;
            else if (op == '-') opcode = 2;
            else if (op == '*') opcode = 3;
            else if (op == '/') opcode = 4;
        }
        //matching assignment operation
        else if (sscanf(programLine, " x%d = %d", &dest, &src1) == 2) {
            opcode = 7; src2 = 0;
        } 
        else {
            invalid = 1;
            printf("Invalid instruction found. Cannot perform operation\n");
            break;
        }

        if ((dest < 0 || dest > 255) || (src1 < 0 || src1 > 255) || (src2 < 0 || src2 > 255)) {
            printf("Invalid operation. Input is out of range\n");
            outOfRange = 1;
            break;
        } 

        char bytes[4] = { opcode, dest, src1, src2 };
        fwrite(bytes, sizeof(char), 4, output);
        
    }

    //override output file if an operation is found out to be invalid
    if(invalid || outOfRange) {
        output = fopen(outputFile, "wb");
    }

    //Writing halt instruction at eof
    char halt[4] = { 0, 0, 0, 0 };
    fwrite(halt, sizeof(char), 4, output);

    fclose(input);
    fclose(output);
}