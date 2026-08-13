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

// Vector Operations
#define OP_VLOAD_CONST    0x30  // Load scalar constant into vector: v_dest = const
#define OP_VADD_VAR       0x31  // Vector addition: v_dest = v_src1 + v_src2
#define OP_VSUB_VAR       0x32  // Vector subtraction: v_dest = v_src1 - v_src2
#define OP_VMUL_VAR       0x33  // Vector multiplication: v_dest = v_src1 * v_src2
#define OP_VEXTRACT       0x34  // Extract vector lane: x_dest = v_src1[lane]

#endif