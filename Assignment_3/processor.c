#include <stdio.h>
#include "processor.h"
#include "memory.h" //needed to access instruction and data array
#include "opcodes.h"

int VRegister[32][8];
int Register[256];
int PC;
int opcode, dest, src1, src2; 
int end_of_simulation = 0;

//condition flags
int Z, N, C, V;

void reset(void) {
    for (int i = 0; i < 256; i++) {
        Register[i] = 0;
    }
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 8; j++) 
            VRegister[i][j] = 0;
    }

    PC = 0;
    Z = N = C = V = 0; 
    end_of_simulation = 0;
}

void fetch(void) {
    //values fetched from current instruction
    opcode = (unsigned char)Instruction[PC];
    dest   = (unsigned char)Instruction[PC+1];
    src1   = (unsigned char)Instruction[PC+2];
    src2   = (unsigned char)Instruction[PC+3];
    PC += 4;
    printf("%X %X %X %X\n", opcode, dest, src1, src2); 

}

void printVectorRegisters(void) {
    printf("\nVector Registers\n");
    for (int i = 0; i < 32; i++) {
        int non_zero = 0;
        
        for (int j = 0; j < 8; j++) {
            if (VRegister[i][j] != 0) {
                non_zero = 1;
                break; 
            }
        }        
        if (non_zero) {
            for (int j = 0; j < 8; j++) {
                printf("%d ", VRegister[i][j]);
            }
            printf("\n");
        }
    }
}

void decode(void) {}

// Updates Z, N, C, V after an ADD operation
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

//Updates Z, N, C, V after a SUBTRACT operation
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

//Updates Z, N, C, V after a branch condition 
static int checkCondition(int code) {
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

void execute(void) {
    switch (opcode) {
        case OP_ADD_VAR: { //Add
            int op1 = Register[src1], op2 = Register[src2];
            int result = op1 + op2;
            Register[dest] = result;
            updateFlagsAdd(op1, op2, result);
            break;
        }

        case OP_SUB_VAR: { //Subtract 
            int op1 = Register[src1], op2 = Register[src2];
            int result = op1 - op2;
            Register[dest] = result;
            updateFlagsSub(op1, op2, result);
            break;
        }

        case OP_ADD_CONST:  {
            int op1 = Register[src1], op2 = src2;
            int result = op1 + op2;
            Register[dest] = result;
            updateFlagsAdd(op1, op2, result);
            break;
        }
        case OP_SUB_CONST: {
            int op1 = Register[src1], op2 = src2;
            int result = op1 - op2;
            Register[dest] = result;
            updateFlagsSub(op1, op2, result);
            break;
        }

        case OP_MUL_VAR: //Multiply
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
            
        case OP_MEMREAD_VAR: //x_dest = [x_src2]  (address held in register src1)
            Register[dest] = Data[Register[src2]];
            break;

        case OP_MEMREAD_CONST: //x_dest = [const]  (src2 IS the literal address)
            Register[dest] = Data[src2];
            break;
 
        case OP_MEMWRITE_VAR: //[x_dest] = x_src2 (address held in register dest)
            Data[Register[dest]] = Register[src2];
            break;

        case OP_MEMWRITE_CONST: //[const] = x_src2 (dest IS the literal address)
            Data[dest] = Register[src2];
            break;
 
        case OP_DATAMOVE_CONST: //x_dest = constant  (constant held in src1)
            Register[dest] = src1;
            break;

        case OP_DATAMOVE_VAR: //x_dest = x_src1 
            Register[dest] = Register[src1];
            break;

        case OP_VECTOR_ADD_VAR_VEC: //add two vecotrs
            for (int i = 0; i < 8; i++) {
                VRegister[dest][i] = VRegister[src1][i] + VRegister[src2][i];
            }
            break;

        case OP_VECTOR_SUB_VAR_VEC: //subtract 2 vectors
            for (int i = 0; i < 8; i++) {
                VRegister[dest][i] = VRegister[src1][i] - VRegister[src2][i];
            }
            break;

        case OP_VECTOR_MUL_VAR_VEC: //multiply 2 vectors
            for (int i = 0; i < 8; i++) {
                VRegister[dest][i] = VRegister[src1][i] * VRegister[src2][i];
            }
            break;
        
        case OP_VECTOR_ADD_CONST: //add a vector and constant
            for (int i = 0; i < 8; i++) {
                VRegister[dest][i] = VRegister[src1][i] + src2;
            }
            break;

        case OP_VECTOR_SUB_CONST: //subtract a vector and constant
            for (int i = 0; i < 8; i++) {
                VRegister[dest][i] = VRegister[src1][i] - src2;
            }
            break;

        case OP_VECTOR_MUL_CONST: //multiply a vector and constant
            for (int i = 0; i < 8; i++) {
                VRegister[dest][i] = VRegister[src1][i] * src2;
            }
            break;
        
        case OP_VECTOR_ADD_VAR_REG:  //add a vector and register val
            for (int i = 0; i < 8; i++) {
                VRegister[dest][i] = VRegister[src1][i] + Register[src2];
            }
            break;

        case OP_VECTOR_SUB_VAR_REG: //subtract a vector and register val
            for (int i = 0; i < 8; i++) {
                VRegister[dest][i] = VRegister[src1][i] - Register[src2];
            }
            break;

        case OP_VECTOR_MUL_VAR_REG: //multiply a vector and register val
            for (int i = 0; i < 8; i++) {
                VRegister[dest][i] = VRegister[src1][i] * Register[src2];
            }
            break;
       

        case OP_VECTOR_READ_VAR: //read from vector and store in [x]
            for (int i = 0; i < 8; i++, dest = dest + 4) {
                Data[Register[dest]] = VRegister[src2][i];
            }
            break;

        case OP_VECTOR_WRITE_VAR:  //write into vector from [x]
            for (int i = 0; i < 8; i++, src2 = src2 + 4){
                VRegister[dest][i] = Data[Register[src2]];
            }
            break;     

        case OP_VECTOR_READ_CONST: //read from vector and store in [const]
            for (int i = 0; i < 8; i++, dest = dest + 4) {
                Data[dest] = VRegister[src2][i];
            }
            break;

        case OP_VECTOR_WRITE_CONST: //write into vector from [const]
            for (int i = 0, tmp = src2; i < 8; i++, tmp = tmp + 4) {
                VRegister[dest][i] = Data[tmp];
            }
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
                    /* address of this branch instruction itself */
                    int instrAddr = PC - 4;
                    PC = instrAddr + offset * 4;
                }
            } else {
                fprintf(stderr, "Unknown opcode %d at PC=%d\n", opcode, PC - 4);
                end_of_simulation = 1;
            }
            break;
    }
}