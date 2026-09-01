#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "memory.h"

char memory[MEMSIZE];
int  pageTable[NP][NUM_LOGICAL_PAGES];
char freeFrames[NUM_PHYSICAL_PAGES];

void memoryInit(void) {
    memset(memory, 0, sizeof(memory));
 
    for (int p = 0; p < NP; p++)
        for (int i = 0; i < NUM_LOGICAL_PAGES; i++)
            pageTable[p][i] = -1;
 
    memset(freeFrames, 0, sizeof(freeFrames));
    freeFrames[0] = 1; // frame 0 is reserved, never handed out 
}

int getFreeFrame(void) {
    for (int i = 1; i < NUM_PHYSICAL_PAGES; i++) { //skip reserved frame 0
        if (!freeFrames[i]) {
            freeFrames[i] = 1;
            return i;
        }
    }
    printf("ERROR: out of physical memory frames\n");
    return -1;
}

int getPhysicalAddress(int proc_id, int isFetch, int address) {
    int pageIndex = isFetch ? (address / PAGESIZE) : (NUM_INSTR_PAGES + address / PAGESIZE);
 
    if (pageIndex < 0 || pageIndex >= NUM_LOGICAL_PAGES) {
        printf("ERROR: logical address %d out of range for process %d\n", address, proc_id);
        return 0;
    }
 
    int frame = pageTable[proc_id][pageIndex];
    if (frame < 0) {
        printf("ERROR: page fault - unmapped page %d for process %d\n", pageIndex, proc_id);
        return 0;
    }   
    return frame * PAGESIZE + (address % PAGESIZE);
}

static void mapAndCopy(int proc_id, int logicalPage, const char *src, int len) {
    int frame = getFreeFrame();
    if (frame < 0) {
        exit(1);
    }
    pageTable[proc_id][logicalPage] = frame;
    if (len > 0) memcpy(&memory[frame * PAGESIZE], src, len);
}

void initialize(int proc_id, const char *programFile, const char *dataFile) {
    
    static char progBuf[INSTR_MEM_SIZE];
    static char dataBuf[DATA_MEM_SIZE];
    memset(progBuf, 0, sizeof(progBuf));
    memset(dataBuf, 0, sizeof(dataBuf));
 
    FILE *program = fopen(programFile, "rb");
    if (program) {
        fread(progBuf, sizeof(char), INSTR_MEM_SIZE, program);
        fclose(program);
    } else {
        printf("%s not found, Instruction memory left as 0 for proc %d.\n", programFile, proc_id);
    }
 
    if (dataFile) {
        FILE *dataFp = fopen(dataFile, "r");
        if (dataFp) {
            int addr = 0;
            unsigned int val;
            while (addr < DATA_MEM_SIZE && fscanf(dataFp, "%x", &val) == 1) { 
                dataBuf[addr++] = (unsigned char)val;
            }
            fclose(dataFp);
        }
    }
 
    /* One frame per instruction page. */
    for (int p = 0; p < NUM_INSTR_PAGES; p++) {
        int offset = p * PAGESIZE;
        int len = PAGESIZE;
        if (offset + len > INSTR_MEM_SIZE) len = INSTR_MEM_SIZE - offset;
        mapAndCopy(proc_id, p, progBuf + offset, len);
    }
 
    for (int p = 0; p < NUM_DATA_PAGES; p++) {
        int off = p * PAGESIZE;
        int len = PAGESIZE;
        if (off + len > DATA_MEM_SIZE) len = DATA_MEM_SIZE - off;
        mapAndCopy(proc_id, NUM_INSTR_PAGES + p, dataBuf + off, len);
    }
}

void finalize(int proc_id, const char *dataFile) {
    if (dataFile) {
        static char dataBuf[DATA_MEM_SIZE];
        memset(dataBuf, 0, sizeof(dataBuf));
 
        for (int p = 0; p < NUM_DATA_PAGES; p++) {
            int frame = pageTable[proc_id][NUM_INSTR_PAGES + p];
            if (frame < 0) continue; 
            int offset = p * PAGESIZE;
            int len = PAGESIZE;
            if (offset + len > DATA_MEM_SIZE) len = DATA_MEM_SIZE - offset;
            memcpy(dataBuf + offset, &memory[frame * PAGESIZE], len);
        }
 
        FILE *data = fopen(dataFile, "w");
        if (!data) {
            printf("Error: cannot write %s\n", dataFile);
        } else {
            fwrite(dataBuf, sizeof(char), DATA_MEM_SIZE, data);
            fclose(data);
        }
    }
 
    for (int p = 0; p < NUM_LOGICAL_PAGES; p++) {
        int frame = pageTable[proc_id][p];
        if (frame >= 0) {
            freeFrames[frame] = 0;
            pageTable[proc_id][p] = -1;
        }
    }
}
