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

// Load / Store Instructions

// Word (32-bit / 4 bytes)
#define OP_LOAD_W_VAR     0x40  // x_dest = [x_src]
#define OP_LOAD_W_CONST   0x41  // x_dest = [const]
#define OP_STORE_W_VAR    0x42  // [x_dest] = x_src
#define OP_STORE_W_CONST  0x43  // [const]  = x_src

// Half-Word (16-bit / 2 bytes)
#define OP_LOAD_HW_VAR    0x44  // x_dest = h[x_src]
#define OP_LOAD_HW_CONST  0x45  // x_dest = h[const]
#define OP_STORE_HW_VAR   0x46  // h[x_dest] = x_src
#define OP_STORE_HW_CONST 0x47  // h[const]  = x_src

// Byte (8-bit / 1 byte)
#define OP_LOAD_B_VAR     0x48  // x_dest = b[x_src]
#define OP_LOAD_B_CONST   0x49  // x_dest = b[const]
#define OP_STORE_B_VAR    0x4A  // b[x_dest] = x_src
#define OP_STORE_B_CONST  0x4B  // b[const]  = x_src

// Half-Byte / Nibble (4-bit)
#define OP_LOAD_HB_VAR    0x4C  // x_dest = hb[x_src]
#define OP_LOAD_HB_CONST  0x4D  // x_dest = hb[const]
#define OP_STORE_HB_VAR   0x4E  // hb[x_dest] = x_src
#define OP_STORE_HB_CONST 0x4F  // hb[const]  = x_src

#endif