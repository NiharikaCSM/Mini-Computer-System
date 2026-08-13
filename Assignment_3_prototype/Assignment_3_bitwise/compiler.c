#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "compiler.h"
#include "opcodes.h"

static int invalid = 0, outOfRange = 0;

#define MAX_LINES     1000
#define MAX_LINE_LEN  256
#define MAX_LABELS    256
#define MAX_LABEL_LEN 64

typedef struct {
    char name[MAX_LABEL_LEN]; //name of the label
    int index; //index at which we should jump to if label is encountered
} Label;

static Label labels[MAX_LABELS]; //array containing label struct elements
static int labelCount = 0;

/* Condition-code suffixes, in the order of their branch-table codes (0-14) */
static const char *suffixes[] = {
    "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
    "hi", "ls", "ge", "lt", "gt", "le", "al"
};
#define NUM_SUFFIXES (sizeof(suffixes) / sizeof(suffixes[0]))

//strips comments and makes instructions lowercase
static void cleanUpLine(char *line){
    char *comment = strchr(line, '%');
    if (comment) *comment = '\0';
    for (char *p = line; *p; p++) {
        *p = (char)tolower((unsigned char)*p);
    }
}

// returns 1 if every character is whitespace (or the line is empty)
static int isBlankLine(const char *line) {
    for (const char *p = line; *p; p++) {
        if (!isspace((unsigned char)*p)) {
            return 0;
        }
    }
    return 1;
}

//parses the name of the label
static int parseLabelLine(const char *programLine, char *labelName) {
    if (programLine[0] != '.') return 0;

    int i = 1, j = 0;
    while (programLine[i] && isalnum((unsigned char)programLine[i]) && j < MAX_LABEL_LEN - 1) {
        labelName[j++] = (char)tolower((unsigned char)programLine[i]);
        i++;
    }
    labelName[j] = '\0';
    return j > 0;
}

//find the label in the labels array to get the index to jump to
static int findLabel(const char *name) {
    for (int i = 0; i < labelCount; i++) {
        if (strcmp(labels[i].name, name) == 0) return labels[i].index;
    }
    return -1;
}

//get code of branch suffix to add to opcode
static int suffixCode(const char *s) {
    for (int i = 0; i < (int)NUM_SUFFIXES; i++) {
        if (strcmp(suffixes[i], s) == 0) return i;
    }
    return -1;
}

static int inRange(int v) {
    return v >= 0 && v <= 255;
}

void compile(const char *inputFile, const char *outputFile) {

    FILE *input = fopen(inputFile, "r");
    if (!input) exit(1);

    //storing all instructions in lines array because we'll make multiple passes on it
    static char lines[MAX_LINES][MAX_LINE_LEN];
    int lineCount = 0;

    while (lineCount < MAX_LINES && fgets(lines[lineCount], MAX_LINE_LEN, input)) {
        lineCount++;
    }
    fclose(input);

    invalid = 0;
    outOfRange = 0;
    labelCount = 0;

    /* ---------- Pass 1: clean each line, collect label  ---------- */
    int instrIndex = 0;
    for (int i = 0; i < lineCount; i++) {
        cleanUpLine(lines[i]);          
        if (isBlankLine(lines[i])) continue;   

        char labelName[MAX_LABEL_LEN];
        if (parseLabelLine(lines[i], labelName)) {
            if (labelCount < MAX_LABELS) {
                strncpy(labels[labelCount].name, labelName, MAX_LABEL_LEN - 1);
                labels[labelCount].name[MAX_LABEL_LEN - 1] = '\0';
                labels[labelCount].index = instrIndex;
                labelCount++;
            }
            continue; 
        }
        instrIndex++;
    }

    FILE *output = fopen(outputFile, "wb");
    if (!output)
        exit(1);

    /* ---------- Pass 2: parse each instruction  ---------- */
    instrIndex = 0;

    for (int i = 0; i < lineCount; i++) {
        if (invalid || outOfRange) break;

        char *programLine = lines[i]; //already cleaned in pass 1
        if (isBlankLine(programLine)) continue;

        char labelName[MAX_LABEL_LEN];
        if (parseLabelLine(programLine, labelName)) continue; //already recorded

        int opcode = 0, dest = 0, src1 = 0, src2 = 0;
        int matched = 0;
        char op;
        char suffixBuffer[8], labelBuffer[MAX_LABEL_LEN];

        //branch: B<suffix> .label
        if (!matched && sscanf(programLine, " b%2s .%63s", suffixBuffer, labelBuffer) == 2) {
            int code = suffixCode(suffixBuffer);
            if (code >= 0) {
                int target = findLabel(labelBuffer);
                if (target < 0) {
                    printf("%d %d", code, target);
                    printf("Invalid instruction found. Cannot perform operation\n");
                    invalid = 1;
                    break;
                }
                int offset = target - instrIndex;
                if (offset < -128 || offset > 127) {
                    printf("Invalid operation. Input is out of range\n");
                    outOfRange = 1;
                    break;
                }
                opcode = OP_BRANCH_BASE + code;
                dest = 0;
                src1 = 0;
                src2 = (offset < 0) ? offset + 256 : offset;
                matched = 1;
            }
        }

        //legacy read: address is a constant 
        if (!matched && sscanf(programLine, " read x%d, %d", &dest, &src1) == 2) {
            opcode = OP_MEMREAD_CONST;
            src2 = 0;
            matched = 1;
        }

        //legacy write: address is a constant 
        if (!matched && sscanf(programLine, " write x%d, %d", &src1, &dest) == 2) {
            opcode = OP_MEMWRITE_CONST;
            src2 = 0;
            matched = 1;
        }

        //memory read, address held in a register
        if (!matched && sscanf(programLine, " x%d = [x%d]", &dest, &src2) == 2) {
            opcode = OP_MEMREAD_VAR;
            src1 = 0;
            matched = 1;
        }

        //memory read, address is a literal constant
        if (!matched && sscanf(programLine, " x%d = [%d]", &dest, &src2) == 2) {
            opcode = OP_MEMREAD_CONST;
            src1 = 0;
            matched = 1;
        }

        //memory write, address held in a register
        if (!matched && sscanf(programLine, " [x%d] = x%d", &dest, &src2) == 2) {
            opcode = OP_MEMWRITE_VAR;
            src1 = 0;
            matched = 1;
        }

        //memory write, address is a literal constant
        if (!matched && sscanf(programLine, " [%d] = x%d", &dest, &src2) == 2) {
            opcode = OP_MEMWRITE_CONST;
            src1 = 0;
            matched = 1;
        }

        //arithmetic, 2 variable operands
        if (!matched && sscanf(programLine, " x%d = x%d %c x%d", &dest, &src1, &op, &src2) == 4) {
            int oc = 0;
            if      (op == '+') oc = OP_ADD_VAR;
            else if (op == '-') oc = OP_SUB_VAR;
            else if (op == '*') oc = OP_MUL_VAR;
            else if (op == '/') oc = OP_DIV_VAR;
            if (oc) { opcode = oc; matched = 1; }
        }

        //arithmetic, second operand is a constant
        if (!matched && sscanf(programLine, " x%d = x%d %c %d", &dest, &src1, &op, &src2) == 4) {
            int oc = 0;
            if      (op == '+') oc = OP_ADD_CONST;
            else if (op == '-') oc = OP_SUB_CONST;
            else if (op == '*') oc = OP_MUL_CONST;
            else if (op == '/') oc = OP_DIV_CONST;
            if (oc) { opcode = oc; matched = 1; }
        }

        // Bitwise shifts (Variable): x1 = x2 << x3, x1 = x2 >> x3
        if (!matched && sscanf(programLine, " x%d = x%d %2s x%d", &dest, &src1, suffixBuffer, &src2) == 4) {
            if (strcmp(suffixBuffer, "<<") == 0)      { opcode = OP_LSHIFT_VAR; matched = 1; }
            else if (strcmp(suffixBuffer, ">>") == 0) { opcode = OP_RSHIFT_VAR; matched = 1; }
        }

        // Bitwise shifts (Constant): x1 = x2 << 4, x1 = x2 >> 2
        if (!matched && sscanf(programLine, " x%d = x%d %2s %d", &dest, &src1, suffixBuffer, &src2) == 4) {
            if (strcmp(suffixBuffer, "<<") == 0)      { opcode = OP_LSHIFT_CONST; matched = 1; }
            else if (strcmp(suffixBuffer, ">>") == 0) { opcode = OP_RSHIFT_CONST; matched = 1; }
        }

        // Bitwise logic (Variable): x1 = x2 & x3, x1 = x2 | x3, x1 = x2 ^ x3
        if (!matched && sscanf(programLine, " x%d = x%d %c x%d", &dest, &src1, &op, &src2) == 4) {
            int oc = 0;
            if      (op == '&') oc = OP_AND_VAR;
            else if (op == '|') oc = OP_OR_VAR;
            else if (op == '^') oc = OP_XOR_VAR;
            if (oc) { opcode = oc; matched = 1; }
        }

        // Bitwise logic (Constant): x1 = x2 & 15, x1 = x2 | 255, x1 = x2 ^ 0xFF
        if (!matched && sscanf(programLine, " x%d = x%d %c %d", &dest, &src1, &op, &src2) == 4) {
            int oc = 0;
            if      (op == '&') oc = OP_AND_CONST;
            else if (op == '|') oc = OP_OR_CONST;
            else if (op == '^') oc = OP_XOR_CONST;
            if (oc) { opcode = oc; matched = 1; }
        }

        // Arithmetic Right Shift (Variable): x1 = x2 >>> x3
        if (!matched && sscanf(programLine, " x%d = x%d %3s x%d", &dest, &src1, suffixBuffer, &src2) == 4) {
            if (strcmp(suffixBuffer, ">>>") == 0) { 
                opcode = OP_ASR_VAR; 
                matched = 1; 
            }
        }

        // Arithmetic Right Shift (Constant): x1 = x2 >>> 2
        if (!matched && sscanf(programLine, " x%d = x%d %3s %d", &dest, &src1, suffixBuffer, &src2) == 4) {
            if (strcmp(suffixBuffer, ">>>") == 0) { 
                opcode = OP_ASR_CONST; 
                matched = 1; 
            }
        }

        //data movement: x_dest = constant
        if (!matched && sscanf(programLine, " x%d = %d", &dest, &src1) == 2) {
            opcode = OP_DATAMOVE_CONST;
            src2 = 0;
            matched = 1;
        }

        //data movement: x_dest = constant
        if (!matched && sscanf(programLine, " x%d = x%d", &dest, &src1) == 2) {
            opcode = OP_DATAMOVE_VAR;
            src2 = 0;
            matched = 1;
        }

        // Bitwise NOT: x1 = ~x2
        if (!matched && sscanf(programLine, " x%d = ~x%d", &dest, &src1) == 2) {
            opcode = OP_NOT;
            src2 = 0;
            matched = 1;
        }

        

        if (!matched) {
            printf("Invalid instruction found. Cannot perform operation\n");
            invalid = 1;
            break;
        }

        if (!inRange(dest) || !inRange(src1) || !inRange(src2)) {
            printf("Invalid operation. Input is out of range\n");
            outOfRange = 1;
            break;
        }

        unsigned char bytes[4] = {
            (unsigned char)opcode, (unsigned char)dest,
            (unsigned char)src1,   (unsigned char)src2
        };
        fwrite(bytes, sizeof(unsigned char), 4, output);

        instrIndex++;
    }

    //override output file if an operation is found out to be invalid
    if (invalid || outOfRange) {
        fclose(output);
        output = fopen(outputFile, "wb");
        if (!output) exit(1);
    }

    //Writing halt instruction at eof 
    unsigned char halt[4] = { 0, 0, 0, 0 };
    fwrite(halt, sizeof(unsigned char), 4, output);

    fclose(output);
}