#include <stdio.h>
#include "processor.h"
#include "memory.h" 
#include "opcodes.h"

int VRegister[NP][32][8];
int Register[NP][256];
int PC[NP];
int end_of_simulation[NP];

//condition flags per processor
int Zf[NP], Nf[NP], Cf[NP], Vf[NP];

static int opcode_a[NP], dest_a[NP], src1_a[NP], src2_a[NP];

static FILE *fd_log;

void resetProcessor(int proc_id) {
    for (int i = 0; i < 256; i++) {
        Register[proc_id][i] = 0;
    }
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 8; j++)
            VRegister[proc_id][i][j] = 0;
    }

    PC[proc_id] = 0;
    Zf[proc_id] = Nf[proc_id] = Cf[proc_id] = Vf[proc_id] = 0;
    end_of_simulation[proc_id] = 0;

    if (!fd_log) {
        fd_log = fopen("log.txt", "a");
    }
}

void closeProcessorLog(void) {
     if (fd_log) {
        fclose(fd_log);
        fd_log = NULL;
    }
}

void fetch(int proc_id) {
    int pc = PC[proc_id];
    int physAddr = getPhysicalAddress(proc_id, 1, pc);
    opcode_a[proc_id] = (unsigned char)memory[physAddr];
    dest_a[proc_id]   = (unsigned char)memory[physAddr + 1];
    src1_a[proc_id]   = (unsigned char)memory[physAddr + 2];
    src2_a[proc_id]   = (unsigned char)memory[physAddr + 3];
    PC[proc_id] += 4;
}

void printVectorRegisters(int proc_id) {
    printf("\nVector Registers (proc %d)\n", proc_id);
    for (int i = 0; i < 32; i++) {
        int non_zero = 0;

        for (int j = 0; j < 8; j++) {
            if (VRegister[proc_id][i][j] != 0) {
                non_zero = 1;
                break;
            }
        }
        if (non_zero) {
            for (int j = 0; j < 8; j++) {
                printf("%d ", VRegister[proc_id][i][j]);
            }
            printf("\n");
        }
    }
}

void decode(int proc_id) { (void)proc_id; }

// Updates Z, N, C, V after an ADD operation
static void updateFlagsAdd(int proc_id, int op1, int op2, int result) {
    Zf[proc_id] = (result == 0) ? 1 : 0;
    Nf[proc_id] = (result < 0) ? 1 : 0;

    unsigned int uOp1 = (unsigned int)op1;
    unsigned int uOp2 = (unsigned int)op2;
    unsigned int uResult = (unsigned int)result;
    Cf[proc_id] = (uResult < uOp1 || uResult < uOp2) ? 1 : 0;

    int signOp1 = (op1 < 0) ? 1 : 0;
    int signOp2 = (op2 < 0) ? 1 : 0;
    int signResult = (result < 0) ? 1 : 0;
    Vf[proc_id] = (signOp1 == signOp2 && signResult != signOp1) ? 1 : 0;
}

//Updates Z, N, C, V after a SUBTRACT operation
static void updateFlagsSub(int proc_id, int op1, int op2, int result) {
    Zf[proc_id] = (result == 0) ? 1 : 0;
    Nf[proc_id] = (result < 0) ? 1 : 0;

    unsigned int uOp1 = (unsigned int)op1;
    unsigned int uOp2 = (unsigned int)op2;
    Cf[proc_id] = (uOp1 > uOp2) ? 1 : 0;

    int signOp1 = (op1 < 0) ? 1 : 0;
    int signOp2 = (op2 < 0) ? 1 : 0;
    int signResult = (result < 0) ? 1 : 0;
    Vf[proc_id] = (signOp1 != signOp2 && signResult == signOp2) ? 1 : 0;
}

//Checks a branch condition against this processor's flags
static int checkCondition(int proc_id, int code) {
    int Z = Zf[proc_id], N = Nf[proc_id], C = Cf[proc_id], V = Vf[proc_id];
    switch (code) {
        case 0:  return Z == 1;                  //EQ
        case 1:  return Z == 0;                  //NE
        case 2:  return C == 1;                  //CS
        case 3:  return C == 0;                  //CC
        case 4:  return N == 1;                  //MI
        case 5:  return N == 0;                  //PL
        case 6:  return V == 1;                  //VS
        case 7:  return V == 0;                  //VC
        case 8:  return (C == 1 && Z == 0);       //HI
        case 9:  return (C == 0 || Z == 1);       //LS
        case 10: return (N == V);                 //GE
        case 11: return (N != V);                 //LT
        case 12: return (Z == 0 && (N == V));      //GT
        case 13: return (Z == 1 || (N != V));      //LE
        case 14: return 1;                        //AL
        default: return 0;
    }
}

void execute(int proc_id) {
    int opcode = opcode_a[proc_id];
    int dest   = dest_a[proc_id];
    int src1   = src1_a[proc_id];
    int src2   = src2_a[proc_id];

    switch (opcode) {
        case OP_ADD_VAR: { //Add
            int op1 = Register[proc_id][src1], op2 = Register[proc_id][src2];
            int result = op1 + op2;
            Register[proc_id][dest] = result;
            updateFlagsAdd(proc_id, op1, op2, result);
            break;
        }

        case OP_SUB_VAR: { //Subtract
            int op1 = Register[proc_id][src1], op2 = Register[proc_id][src2];
            int result = op1 - op2;
            Register[proc_id][dest] = result;
            updateFlagsSub(proc_id, op1, op2, result);
            break;
        }

        case OP_ADD_CONST:  {
            int op1 = Register[proc_id][src1], op2 = src2;
            int result = op1 + op2;
            Register[proc_id][dest] = result;
            updateFlagsAdd(proc_id, op1, op2, result);
            break;
        }
        case OP_SUB_CONST: {
            int op1 = Register[proc_id][src1], op2 = src2;
            int result = op1 - op2;
            Register[proc_id][dest] = result;
            updateFlagsSub(proc_id, op1, op2, result);
            break;
        }

        case OP_MUL_VAR: //Multiply
            Register[proc_id][dest] = Register[proc_id][src1] * Register[proc_id][src2];
            break;

        case OP_MUL_CONST:
            Register[proc_id][dest] = Register[proc_id][src1] * src2;
            break;

        case OP_DIV_VAR:
            if (Register[proc_id][src2] == 0)
                printf("Error : Cannot divide by zero\n");
            else
                Register[proc_id][dest] = Register[proc_id][src1] / Register[proc_id][src2];
            break;

        case OP_DIV_CONST:
            if (src2 == 0)
                printf("Error : Cannot divide by zero\n");
            else
                Register[proc_id][dest] = Register[proc_id][src1] / src2;
            break;

        case OP_MEMREAD_VAR: {//x_dest = [x_src2]  (address held in register src1)
            int addr = Register[proc_id][src2];
            int phys = getPhysicalAddress(proc_id, 0, addr);
            Register[proc_id][dest] = memory[phys];
            break;
        }

        case OP_MEMREAD_CONST: {//x_dest = [const]  (src2 IS the literal address)
            int phys = getPhysicalAddress(proc_id, 0, src2);
            Register[proc_id][dest] = memory[phys];
            break;
        }

        case OP_MEMWRITE_VAR: {//[x_dest] = x_src2 (address held in register dest)
            int addr = Register[proc_id][dest];
            int phys = getPhysicalAddress(proc_id, 0, addr);
            memory[phys] = (char)Register[proc_id][src2];
            break;
        }

        case OP_MEMWRITE_CONST: {//[const] = x_src2 (dest IS the literal address)
            int phys = getPhysicalAddress(proc_id, 0, dest);
            memory[phys] = (char)Register[proc_id][src2];
            break;
        }
        case OP_DATAMOVE_CONST: //x_dest = constant  (constant held in src1)
            Register[proc_id][dest] = src1;
            break;

        case OP_DATAMOVE_VAR: //x_dest = x_src1
            Register[proc_id][dest] = Register[proc_id][src1];
            break;
        
        //writes to processor's log file
         case OP_PRINT: {
            int regNum = src2;
            if (fd_log) {
                fprintf(fd_log, "Process id: %d x%d : %X\n",
                        proc_id, regNum, Register[proc_id][regNum]);
                fflush(fd_log);
            }
            break;
        }

        case OP_VECTOR_ADD_VAR_VEC: //add two vectors
            for (int i = 0; i < 8; i++) {
                VRegister[proc_id][dest][i] = VRegister[proc_id][src1][i] + VRegister[proc_id][src2][i];
            }
            break;

        case OP_VECTOR_SUB_VAR_VEC: //subtract 2 vectors
            for (int i = 0; i < 8; i++) {
                VRegister[proc_id][dest][i] = VRegister[proc_id][src1][i] - VRegister[proc_id][src2][i];
            }
            break;

        case OP_VECTOR_MUL_VAR_VEC: //multiply 2 vectors
            for (int i = 0; i < 8; i++) {
                VRegister[proc_id][dest][i] = VRegister[proc_id][src1][i] * VRegister[proc_id][src2][i];
            }
            break;

        case OP_VECTOR_ADD_CONST: //add a vector and constant
            for (int i = 0; i < 8; i++) {
                VRegister[proc_id][dest][i] = VRegister[proc_id][src1][i] + src2;
            }
            break;

        case OP_VECTOR_SUB_CONST: //subtract a vector and constant
            for (int i = 0; i < 8; i++) {
                VRegister[proc_id][dest][i] = VRegister[proc_id][src1][i] - src2;
            }
            break;

        case OP_VECTOR_MUL_CONST: //multiply a vector and constant
            for (int i = 0; i < 8; i++) {
                VRegister[proc_id][dest][i] = VRegister[proc_id][src1][i] * src2;
            }
            break;

        case OP_VECTOR_ADD_VAR_REG:  //add a vector and register val
            for (int i = 0; i < 8; i++) {
                VRegister[proc_id][dest][i] = VRegister[proc_id][src1][i] + Register[proc_id][src2];
            }
            break;

        case OP_VECTOR_SUB_VAR_REG: //subtract a vector and register val
            for (int i = 0; i < 8; i++) {
                VRegister[proc_id][dest][i] = VRegister[proc_id][src1][i] - Register[proc_id][src2];
            }
            break;

        case OP_VECTOR_MUL_VAR_REG: //multiply a vector and register val
            for (int i = 0; i < 8; i++) {
                VRegister[proc_id][dest][i] = VRegister[proc_id][src1][i] * Register[proc_id][src2];
            }
            break;

        case OP_VECTOR_READ_VAR: { //read from vector and store in [x]
            int addr = Register[proc_id][dest];
            for (int i = 0; i < 8; i++, addr += 4) {
                int phys = getPhysicalAddress(proc_id, 0, addr);
                memory[phys] = (char)VRegister[proc_id][src2][i];
            }
            break;
        }

        case OP_VECTOR_WRITE_VAR: { //write into vector from [x]
            int addr = Register[proc_id][src2];
            for (int i = 0; i < 8; i++, addr += 4) {
                int phys = getPhysicalAddress(proc_id, 0, addr);
                VRegister[proc_id][dest][i] = memory[phys];
            }
            break;
        }

        case OP_VECTOR_READ_CONST: {//read from vector and store in [const]
            int addr = dest;
             for (int i = 0; i < 8; i++, addr += 4) {
                int phys = getPhysicalAddress(proc_id, 0, addr);
                memory[phys] = (char)VRegister[proc_id][src2][i];
            }
            break;
        }

        case OP_VECTOR_WRITE_CONST: {//write into vector from [const]
            int addr = src2;
            for (int i = 0; i < 8; i++, addr += 4) {
                int phys = getPhysicalAddress(proc_id, 0, addr);
                VRegister[proc_id][dest][i] = memory[phys];
            }
            break;
        }

         case OP_HALT:
            end_of_simulation[proc_id] = 1;
            break;

        default:
            if (opcode >= OP_BRANCH_BASE && opcode <= (OP_BRANCH_BASE + 14)) {
                int code = opcode - OP_BRANCH_BASE;
                if (checkCondition(proc_id, code)) {
                    /* src2 is an unsigned byte (0-255) that encodes a
                     * signed 8-bit relative offset (two's complement). */
                    int offset = (src2 > 127) ? (src2 - 256) : src2;
                    /* address of this branch instruction itself */
                    int instrAddr = PC[proc_id] - 4;
                    PC[proc_id] = instrAddr + offset * 4;
                }
            } else {
                fprintf(stderr, "Unknown opcode %d at PC=%d (proc %d)\n",
                        opcode, PC[proc_id] - 4, proc_id);
                end_of_simulation[proc_id] = 1;
            }
            break;
    }
}
