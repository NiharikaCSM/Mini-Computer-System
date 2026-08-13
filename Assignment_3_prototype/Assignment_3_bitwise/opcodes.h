// opcodes.h

#ifndef OPCODES_H
#define OPCODES_H

#define OP_HALT           0x00

#define OP_ADD_VAR        0x01
#define OP_SUB_VAR        0x02
#define OP_MUL_VAR        0x03
#define OP_DIV_VAR        0x04
#define OP_MEMREAD_VAR    0x05
#define OP_MEMWRITE_VAR   0x06
#define OP_DATAMOVE_VAR   0x07

#define OP_ADD_CONST      0x09
#define OP_SUB_CONST      0x0A
#define OP_MUL_CONST      0x0B
#define OP_DIV_CONST      0x0C
#define OP_MEMREAD_CONST  0x0D
#define OP_MEMWRITE_CONST 0x0E
#define OP_DATAMOVE_CONST 0x0F

#define OP_BRANCH_BASE    0x10   /* 0x10 + condition code (0-14) */

// Bitwise Operations (Variables)
#define OP_AND_VAR        0x20
#define OP_OR_VAR         0x21
#define OP_XOR_VAR        0x22
#define OP_LSHIFT_VAR     0x23
#define OP_RSHIFT_VAR     0x24

// Bitwise Operations (Constants)
#define OP_AND_CONST      0x25
#define OP_OR_CONST       0x26
#define OP_XOR_CONST      0x27
#define OP_LSHIFT_CONST   0x28
#define OP_RSHIFT_CONST   0x29

// Bitwise Unary Operations
#define OP_NOT            0x2A
#define OP_ASR_VAR        0x2B  // Arithmetic shift right (variable)
#define OP_ASR_CONST      0x2C  // Arithmetic shift right (constant)

#endif