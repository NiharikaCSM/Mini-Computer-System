#ifndef OS_H
#define OS_H

// Sets up task tables, processor bookkeeping, and puts stdin into non-blocking mode for the shell
void osInit(void);

/* Main OS loop: round-robins ready tasks through the scheduler (each
   getting a TIME_SLICE turn), polls the shell for new program names,
   and loads/queues new tasks via the loader. Returns once the user has
   typed "exit" and every ready/waiting task has finished. */
void osRun(void);

#endif
