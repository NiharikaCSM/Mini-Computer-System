#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "os.h"
#include "common.h"
#include "compiler.h"
#include "processor.h"
#include "memory.h"

typedef struct {
    int  pid;
    int  procId;           // -1 until a processor is assigned 
    int  inUse;            // 1 while the task exists (ready or waiting) 
    char progFile[128];    // .txt given at the shell 
    char byteFile[128];    // compiled bytecode
    char dataFile[128];    //initial data (data.byte) 
    int  hasDataFile;
} Task;

static Task tasks[MAX_TASKS];   //array of tasks
static int  nextPid = 1;        //process id counter


static int readyQueue[MAX_TASKS], readyHead = 0, readyTail = 0, readyCount = 0;
static int waitQueue[MAX_TASKS], waitHead = 0, waitTail = 0, waitCount = 0;

static int procBusy[NP];   // 1 if processor i currently belongs to a task
static int shellDone = 0;  // set once the user types "exit"

static void readyPush(int pid) {
    readyQueue[readyTail] = pid;
    readyTail = (readyTail + 1) % MAX_TASKS;
    readyCount++;
}
static int readyPop(void) {
    int pid = readyQueue[readyHead];
    readyHead = (readyHead + 1) % MAX_TASKS;
    readyCount--;
    return pid;
}
static void waitPush(int pid) {
    waitQueue[waitTail] = pid;
    waitTail = (waitTail + 1) % MAX_TASKS;
    waitCount++;
}
static int waitPop(void) {
    int pid = waitQueue[waitHead];
    waitHead = (waitHead + 1) % MAX_TASKS;
    waitCount--;
    return pid;
}

// find an active task in the task array
static Task *findTask(int pid) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].inUse && tasks[i].pid == pid) return &tasks[i];
    }
    return NULL;
}

//find an idle processor
static int findFreeProcessor(void) {
    for (int i = 0; i < NP; i++) {
        if (!procBusy[i]) return i;
    }
    return -1;
}

//run task for required time slice
static void process_instructions(int pid, int count) {
    Task *t = findTask(pid);
    if (!t) return;
    int procId = t->procId;

    for (int i = 0; i < count && !end_of_simulation[procId]; i++) {
        fetch(procId);
        decode(procId);
        execute(procId);
    }
    usleep(10000); 
}

//binds task to processor and initialize memory and processor
static void startTask(Task *t, int procId) {
    t->procId = procId;
    procBusy[procId] = 1;

    initialize(procId, t->byteFile, t->hasDataFile ? t->dataFile : NULL);
    resetProcessor(procId);

    readyPush(t->pid);
}

// create a new task and load it into task array, complile the task file and assign processor
static void loader(const char *progFile, const char *dataFileArg) {
    Task *t = NULL;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!tasks[i].inUse) { t = &tasks[i]; break; }
    }
    if (!t) {
        printf("Too many tasks - cannot load %s\n", progFile);
        return;
    }
 
    t->inUse  = 1;
    t->pid    = nextPid++;
    t->procId = -1;
    strncpy(t->progFile, progFile, sizeof(t->progFile) - 1);
    t->progFile[sizeof(t->progFile) - 1] = '\0';
 
    /* "<base>.txt" -> "<base>.byte" */
    char base[100];
    strncpy(base, progFile, sizeof(base) - 1);
    base[sizeof(base) - 1] = '\0';
    char *dot = strrchr(base, '.');
    if (dot) *dot = '\0';
 
    snprintf(t->byteFile, sizeof(t->byteFile), "%s.byte", base);
 
    if (dataFileArg) {
        strncpy(t->dataFile, dataFileArg, sizeof(t->dataFile) - 1);
        t->dataFile[sizeof(t->dataFile) - 1] = '\0';
    } else {
        t->dataFile[0] = '\0'; // no data file given for this task 
    }
 
    if (compile(progFile, t->byteFile) != 0) {
        printf("Failed to load '%s' - not starting this task.\n", progFile);
        t->inUse = 0;
        return;
    }
 
    t->hasDataFile = 0;
    if (dataFileArg) {
        FILE *df = fopen(t->dataFile, "r");
        t->hasDataFile = (df != NULL);
        if (df) fclose(df);
        if (!t->hasDataFile) {
            printf("Warning: data file '%s' not found - starting '%s' with zeroed data memory.\n", dataFileArg, progFile);
        }
    }
 
    int procId = findFreeProcessor();
    if (procId >= 0) {
        startTask(t, procId);
        printf("Loaded pid %d ('%s') onto processor %d\n", t->pid, progFile, procId);
    } else {
        waitPush(t->pid);
        printf("All processors busy - pid %d ('%s') is waiting\n", t->pid, progFile);
    }
}

static void setStdinNonBlocking(void) {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

//store what the user has typed
static char shellBuf[256];
static int  shellLen = 0;
static int  promptShown = 0;

static void handleShellLine(char *line) {
    int len = (int)strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[--len] = '\0';
    }
    if (len == 0) return;
 
    if (strcmp(line, "exit") == 0) {
        shellDone = 1;
        printf("\nShell: no further input accepted; draining remaining tasks.\n");
        return;
    }

    char progFile[128] = {0};
    char dataFile[128] = {0};
    int n = sscanf(line, "%127s %127s", progFile, dataFile);
 
    if (n == 2) {
        loader(progFile, dataFile);
    } else {
        loader(progFile, NULL);
    }
}

static void shell(void) {
    if (shellDone) return;

    if (!promptShown) {
        printf("$ ");
        fflush(stdout);
        promptShown = 1;
    }

    char c;
    //read user input character by character
    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == '\n') {
            shellBuf[shellLen] = '\0';
            handleShellLine(shellBuf);
            shellLen = 0;
            if (!shellDone) {
                printf("$ ");
                fflush(stdout);
            }
        } else if (shellLen < (int)sizeof(shellBuf) - 1) {
            shellBuf[shellLen++] = c;
        }
    }
}

//scheduler with round robin
static void scheduler(void) {
    int n = readyCount;
    for (int i = 0; i < n; i++) {
        int pid = readyPop();
        Task *t = findTask(pid);
        if (!t) continue;

        process_instructions(pid, TIME_SLICE);

        if (end_of_simulation[t->procId]) {
            finalize(t->procId, t->hasDataFile ? t->dataFile : NULL);
            printf("Task pid %d finished on processor %d\n", t->pid, t->procId);

            int freedProc = t->procId;
            procBusy[freedProc] = 0;
            t->inUse = 0; // task removed from the system 

            //assign processor to a waiting task since one task just got finished
            if (waitCount > 0) {
                int waitingPid = waitPop();
                Task *wt = findTask(waitingPid);
                if (wt) startTask(wt, freedProc);
            }
        } else {
            readyPush(pid); //push back to ready queue after running for time quantum
        }
    }

    shell();
}

void osInit(void) {
    memset(tasks, 0, sizeof(tasks));
    memset(procBusy, 0, sizeof(procBusy));

    readyHead = readyTail = readyCount = 0;
    waitHead  = waitTail  = waitCount  = 0;

    nextPid = 1;
    shellDone = 0;
    shellLen = 0;
    promptShown = 0;
    setStdinNonBlocking();
}

void osRun(void) {
    while (!shellDone || readyCount > 0 || waitCount > 0) {
        scheduler();
    }
    closeProcessorLog();
    printf("All tasks complete. OS shutting down.\n");
}