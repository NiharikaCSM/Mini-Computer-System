#include <stdio.h>
#include "processor.h"
#include "opcodes.h"
#include "bus.h"   //processor now talks to memory only through the bus

int Register[256];
int PC;
int opcode, dest, src1, src2;
int end_of_simulation = 0;

//condition flags
int Z = 0, N = 0, C = 0, V = 0;

void reset(void) {
    for (int i = 0; i < 256; i++) {
        Register[i] = 0;
    }
    PC = 0;
    Z = N = C = V = 0;
    end_of_simulation = 0;
}

void fetch(void) {
    //reading instruction memory thorugh bus
    opcode = bus_read8(BUS_INSTR_BASE + PC);
    dest   = bus_read8(BUS_INSTR_BASE + PC + 1);
    src1   = bus_read8(BUS_INSTR_BASE + PC + 2);
    src2   = bus_read8(BUS_INSTR_BASE + PC + 3);
    PC += 4;
    printf("%d %d %d %d\n", opcode, dest, src1, src2);
}

void decode(void) {}

static void updateFlagsAdd(int op1, int op2, int result) {
    Z = (result == 0) ? 1 : 0;
    N = (result < 0) ? 1 : 0;

    unsigned int uOp1 = (unsigned int)op1;
    unsigned int uOp2 = (unsigned int)op2;
    unsigned int uResult = (unsigned int)result;
    C = (uResult < uOp1 || uResult < uOp2) ? 1 : 0;

    int signOp1 = (op1 < 0) ? 1 : 0;
    int signOp2 = (op2 < 0) ? 1 : 0;
    int signResult = (result < 0) ? 1 : 0;
    V = (signOp1 == signOp2 && signResult != signOp1) ? 1 : 0;
}

static void updateFlagsSub(int op1, int op2, int result) {
    Z = (result == 0) ? 1 : 0;
    N = (result < 0) ? 1 : 0;

    unsigned int uOp1 = (unsigned int)op1;
    unsigned int uOp2 = (unsigned int)op2;
    C = (uOp1 > uOp2) ? 1 : 0;

    int signOp1 = (op1 < 0) ? 1 : 0;
    int signOp2 = (op2 < 0) ? 1 : 0;
    int signResult = (result < 0) ? 1 : 0;
    V = (signOp1 != signOp2 && signResult == signOp2) ? 1 : 0;
}

/* Evaluates a branch condition code (0-14, see Lab 2 branch table) */
static int checkCondition(int code) {
    switch (code) {
        case 0:  return Z == 1;                   //EQ
        case 1:  return Z == 0;                   //NE
        case 2:  return C == 1;                    //CS
        case 3:  return C == 0;                    //CC
        case 4:  return N == 1;                    //MI
        case 5:  return N == 0;                    //PL
        case 6:  return V == 1;                    //VS
        case 7:  return V == 0;                    //VC
        case 8:  return (C == 1 && Z == 0);         //HI
        case 9:  return (C == 0 || Z == 1);         //LS
        case 10: return (N == V);                   //GE
        case 11: return (N != V);                   //LT
        case 12: return (Z == 0 && (N == V));        //GT
        case 13: return (Z == 1 || (N != V));        //LE
        case 14: return 1;                          //AL
        default: return 0;
    }
}

void execute(void) {
    switch (opcode) {
        case OP_ADD_VAR: {
            int op1 = Register[src1], op2 = Register[src2];
            int result = op1 + op2;
            Register[dest] = result;
            updateFlagsAdd(op1, op2, result);
            break;
        }
        case OP_ADD_CONST: {
            int op1 = Register[src1], op2 = src2;
            int result = op1 + op2;
            Register[dest] = result;
            updateFlagsAdd(op1, op2, result);
            break;
        }
        case OP_SUB_VAR: {
            int op1 = Register[src1], op2 = Register[src2];
            int result = op1 - op2;
            Register[dest] = result;
            updateFlagsSub(op1, op2, result);
            break;
        }
        case OP_SUB_CONST: {
            int op1 = Register[src1], op2 = src2;
            int result = op1 - op2;
            Register[dest] = result;
            updateFlagsSub(op1, op2, result);
            break;
        }
        case OP_MUL_VAR:
            Register[dest] = Register[src1] * Register[src2];
            break;
        case OP_MUL_CONST:
            Register[dest] = Register[src1] * src2;
            break;
        case OP_DIV_VAR:
            if (Register[src2] == 0)
                printf("Error : Cannot divide by zero\n");
            else
                Register[dest] = Register[src1] / Register[src2];
            break;
        case OP_DIV_CONST:
            if (src2 == 0)
                printf("Error : Cannot divide by zero\n");
            else
                Register[dest] = Register[src1] / src2;
            break;

        //memory instructions go through the bus
        case OP_MEMREAD_VAR: //x_dest = [x_src1]  (address held in register src1)
            Register[dest] = bus_read8(BUS_DATA_BASE + Register[src1]);
            break;
        case OP_MEMREAD_CONST: //x_dest = [const]  (src1 IS the literal address)
            Register[dest] = bus_read8(BUS_DATA_BASE + src1);
            break;

        case OP_MEMWRITE_VAR: //[x_dest] = x_src1  (address held in register dest)
            bus_write8(BUS_DATA_BASE + Register[dest], (unsigned char)Register[src1]);
            break;
        case OP_MEMWRITE_CONST: //[const] = x_src1  (dest IS the literal address)
            bus_write8(BUS_DATA_BASE + dest, (unsigned char)Register[src1]);
            break;

        case OP_DATAMOVE: //x_dest = constant  (constant held in src1)
            Register[dest] = src1;
            break;

        case OP_HALT:
            end_of_simulation = 1;
            break;

        default:
            if (opcode >= OP_BRANCH_BASE && opcode <= (OP_BRANCH_BASE + 14)) {
                int code = opcode - OP_BRANCH_BASE;
                if (checkCondition(code)) {
                    /* src2 is an unsigned byte (0-255) that encodes a
                     * signed 8-bit relative offset (two's complement). */
                    int offset = (src2 > 127) ? (src2 - 256) : src2;
                    int instrAddr = PC - 4; //address of this branch instruction itself
                    PC = instrAddr + offset * 4;
                }
                /* condition false -> PC already advanced by fetch(), fall through */
            } else {
                fprintf(stderr, "Unknown opcode %d at PC=%d\n", opcode, PC - 4);
                end_of_simulation = 1;
            }
            break;
    }
}
