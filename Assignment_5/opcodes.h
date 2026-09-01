#ifndef OPCODES_H
#define OPCODES_H

#define OP_HALT             0x00

#define OP_ADD_VAR          0x01
#define OP_SUB_VAR          0x02
#define OP_MUL_VAR          0x03
#define OP_DIV_VAR          0x04
#define OP_MEMREAD_VAR      0x05
#define OP_MEMWRITE_VAR     0x06
#define OP_DATAMOVE_VAR     0x07
#define OP_PRINT            0x08

#define OP_ADD_CONST        0x09
#define OP_SUB_CONST        0x0A
#define OP_MUL_CONST        0x0B
#define OP_DIV_CONST        0x0C
#define OP_MEMREAD_CONST    0x0D
#define OP_MEMWRITE_CONST   0x0E
#define OP_DATAMOVE_CONST   0x0F

#define OP_BRANCH_BASE        0x10   /* 0x10 + condition code (0-14) */

#define OP_VECTOR_ADD_VAR_VEC     0x21
#define OP_VECTOR_SUB_VAR_VEC     0x22
#define OP_VECTOR_MUL_VAR_VEC     0x23
#define OP_VECTOR_READ_VAR        0x25
#define OP_VECTOR_WRITE_VAR       0x26
#define OP_VECTOR_ADD_CONST       0x29
#define OP_VECTOR_SUB_CONST       0x2A
#define OP_VECTOR_MUL_CONST       0x2B
#define OP_VECTOR_READ_CONST      0x2C
#define OP_VECTOR_WRITE_CONST     0x2E

#define OP_VECTOR_ADD_VAR_REG     0x31
#define OP_VECTOR_SUB_VAR_REG     0x32
#define OP_VECTOR_MUL_VAR_REG     0x33


#endif
