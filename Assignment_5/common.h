#ifndef COMMON_H
#define COMMON_H

/* ----multiprocessing constants ---- */
#define NP            4     // number of processors, fixed at compile time 
#define TIME_SLICE    10    // instructions run per scheduler turn (time slice) 
#define INSTR_MEM_SIZE 256
#define DATA_MEM_SIZE  4096
#define MAX_TASKS     64    // max simultaneous tasks OS can track 
#define PAGESIZE 512

#define NUM_INSTR_PAGES   ((INSTR_MEM_SIZE + PAGESIZE - 1) / PAGESIZE)
#define NUM_DATA_PAGES    ((DATA_MEM_SIZE  + PAGESIZE - 1) / PAGESIZE)
#define NUM_LOGICAL_PAGES (NUM_INSTR_PAGES + NUM_DATA_PAGES)
 
#define NUM_PHYSICAL_PAGES (NP * NUM_LOGICAL_PAGES + 1)
#define MEMSIZE (NUM_PHYSICAL_PAGES * PAGESIZE)

#define SCHEDULER_SLEEP_USEC 300000

#endif




