#ifndef COMMON_H
#define COMMON_H

/* ----multiprocessing constants ---- */
#define NP            4     // number of processors, fixed at compile time 
#define TIME_SLICE    10    // instructions run per scheduler turn (time slice) 
#define INSTR_MEM_SIZE 256
#define DATA_MEM_SIZE  4096
#define MAX_TASKS     64    // max simultaneous tasks OS can track 

#endif
