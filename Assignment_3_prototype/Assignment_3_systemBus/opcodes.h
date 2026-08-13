// opcodes.h
// Single source of truth for the instruction opcode map (see Lab 2 spec).
// Both compiler.c and processor.c #include this instead of redefining
// the opcode constants themselves, so the two can never drift apart.

#ifndef OPCODES_H
#define OPCODES_H

#define OP_HALT           0x00

#define OP_ADD_VAR        0x01
#define OP_SUB_VAR        0x02
#define OP_MUL_VAR        0x03
#define OP_DIV_VAR        0x04
#define OP_MEMREAD_VAR    0x05
#define OP_MEMWRITE_VAR   0x06
#define OP_DATAMOVE       0x07

#define OP_ADD_CONST      0x09
#define OP_SUB_CONST      0x0A
#define OP_MUL_CONST      0x0B
#define OP_DIV_CONST      0x0C
#define OP_MEMREAD_CONST  0x0D
#define OP_MEMWRITE_CONST 0x0E

#define OP_BRANCH_BASE    0x10   /* 0x10 + condition code (0-14) */

#endif
