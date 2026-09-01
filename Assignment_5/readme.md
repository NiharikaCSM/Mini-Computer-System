# Mini-Computer Simulator — CS527 Lab 5

## Overview

This project simulates a small multiprocessor computer system made up of
five parts:

- **Compiler** (`compiler.c` / `compiler.h`) : reads a program written in a
  simple custom language (integer variables `x0`–`x255`, vector variables
  `v0`–`v31`, arithmetic `+ - * /` with either a variable or constant
  second operand, memory read/write via register-indirect or constant
  addressing, constant assignment, labels, conditional/unconditional
  branches, and `Print`) and translates it into a 4-byte-per-instruction
  bytecode file.


- **Memory** (`memory.c` / `memory.h`) : provides **one shared physical
  memory array**, `memory[MEMSIZE]`, divided into fixed-size 512-byte
  **frames**. Each task's own 256-byte instruction space and 4096-byte
  data space are logical — never accessed directly — and are divided the
  same way into **pages**. Each processor has a page table slot (pageTable[proc_id][...]); whichever process is currently bound to that processor via startTask() has its logical pages mapped there, and that slot is reset and reassigned to a new process once the previous one finishes; a **free-frame list**
  (`freeFrames[...]`) tracks which frames are available, with frame `0`
  permanently reserved and never handed out. `getPhysicalAddress(proc_id,
  isFetch, address)` is the single function every memory access goes
  through to turn a logical address into a physical one.
  `initialize(proc_id, programFile, dataFile)` loads a task's compiled
  bytecode (and, if given, its data file) page by page into freshly
  allocated frames, filling in that process's page table as it goes;
  `finalize(proc_id, dataFile)` writes the final contents of that
  process's data pages back out to the given file when the task halts,
  then returns every frame (instruction and data) it was holding to the
  free list.


- **Processor** (`processor.c` / `processor.h`) : a fetch-decode-execute
  loop, replicated across `NP` processors. Each processor has its own 256
  integer registers, 32 vector registers (8 lanes of 32 bits each), a
  program counter, and its own set of condition flags (`Z N C V`). It
  fetches one 4-byte instruction at a time — translating the program
  counter through the MMU before reading it — executes it (integer/vector
  arithmetic, memory read/write, constant load, branch, or Print),
  translating every memory address it touches through the MMU first,
  updates the flags after every integer add/subtract, and halts when it
  hits an opcode of `0`.


- **Opcodes** (`opcodes.h`) : a single shared header defining every opcode
  constant, included by both `compiler.c` and `processor.c` so the two can
  never drift out of sync with each other.


- **OS** (`os.c` / `os.h`) : a thin operating-system layer sitting on top
  of the processors and the MMU. A **shell** reads program and data
  filenames from the terminal without blocking, a **loader** compiles a
  program and assigns it to a free processor (or queues it if all are
  busy), and a **scheduler** round-robins every active task through a
  fixed time slice on its processor, in a loop, until every task has
  finished and the shell has been told to exit. At start-up, the OS also
  initializes the MMU (`memoryInit()`) — resetting every page table entry
  to "unmapped" and every frame to "free" — before any task can load.

`main.c` is just `osInit(); osRun();`: everything that used to happen
once in `main()` (compiling, loading, resetting, running, finalizing)
happens per-task, driven by the OS, since a program is something the OS
receives from the shell at run time rather than a fixed command-line
argument.

## Virtual memory (paging)

Previously, each processor had its own private, fixed 256-byte instruction array and 4096-byte data array, addressed directly. That's now gone. Instead:

- **Physical memory is one shared array**, `memory[MEMSIZE]`, split into
  fixed-size **frames** of `PAGESIZE` (512) bytes each.
- **Every task's logical address space is also split into pages** of the
  same size: 1 instruction page (256 bytes, rounded up to a page) and 8
  data pages (4096 bytes ÷ 512), for 9 logical pages total per task.
- **Each process has its own page table**, `pageTable[proc_id][logical
  page]`, mapping that logical page to whichever physical frame it's
  currently backed by, or `-1` if it isn't mapped at all.
- **A free-frame list** (`freeFrames[...]`) tracks which of the physical
  frames are currently handed out. **Frame 0 is reserved** and is never
  allocated to anyone, per the assignment's requirement.
- **`getPhysicalAddress(proc_id, isFetch, address)`** is the one place
  logical-to-physical translation happens. `isFetch = 1` selects the
  instruction half of that process's logical space (used only by
  `fetch()`); `isFetch = 0` selects the data half (used by every scalar
  and vector memory read/write). It looks up the frame for that logical
  page in the process's page table and returns `frame * PAGESIZE +
  address % PAGESIZE` — or prints an error and returns `0` on an
  out-of-range address or an unmapped page (a page fault).
- **`initialize()` and `finalize()` are now page-aware.** Loading a task
  claims one free frame per logical page and copies that page's bytes in;
  finishing a task copies its data pages back out to disk and returns
  every frame it held to the free list, so a later task can reuse them.
- **Physical memory is sized generously on purpose.** `common.h` computes
  `NUM_PHYSICAL_PAGES` as `NP * NUM_LOGICAL_PAGES + 1` — enough frames for
  every one of the `NP` processors to have its entire logical space
  mapped in at the same time, plus the one reserved frame. This is a
  deliberate choice: it means the paging *mechanism* (translation,
  allocation, page faults on genuine bugs) gets exercised without the
  simulator hitting spurious "out of physical memory" errors just from
  running a normal number of concurrent tasks.

None of this is visible at the language or opcode level, a `.txt`
program, its compiled bytecode, and the branch/arithmetic/vector opcodes
are all unchanged. Paging only affects how the processor and
OS turn a logical address into a physical one internally.

## Existing features carried over from Lab 4

- **Vector registers and vector arithmetic** — 32 vector registers `v0`–
  `v31`, each holding 8 lanes of 32-bit integers. Vector add/subtract/
  multiply support a second operand that's another vector, a constant, or
  an integer register (broadcast to all 8 lanes). Vector memory
  read/write moves 8 consecutive 32-bit words to/from data memory (now
  translated through the MMU, one element address at a time, since a
  32-byte vector access can span more than one page), with either a
  register-indirect or constant starting address.
- **`Print` instruction** — `print x<n>` writes one line to a shared log
  file: `Process id: <proc_id> x<n> : <value in hex>`. This is currently
  the *only* thing written to the log, nothing else is logged
  automatically.
- **Multiprocessing** — `NP` processors (4 by default, see `common.h`),
  each with fully independent registers, flags, and page table, so
  several compiled programs can run concurrently without interfering with
  each other.
- **A thin OS layer**:
  - **Shell** — reads input from the terminal one character at a time,
    without blocking the rest of the simulator, and only acts once a full
    line (terminated by Enter) has been typed. Typing `exit` stops the
    shell from accepting further input, but already-running tasks keep
    running to completion.
  - **Loader** — takes a line of shell input, compiles the named program,
    and either starts it immediately on a free processor or places it in
    a waiting queue if all processors are currently busy.
  - **Scheduler** — round-robins every currently-running task through a
    fixed time slice (`TIME_SLICE` instructions, see `common.h`) on its
    assigned processor, over and over, promoting a waiting task onto a
    processor (and its own freshly-mapped frames) the moment one frees
    up, until nothing is left to run.
- **Graceful compile failures** — a bad or missing filename typed at the
  shell does not terminate the whole simulator; it prints an error and
  simply refuses that one task, leaving every other running task
  unaffected.

## Language quick reference

```
x5 = 10               % constant assignment
x3 = x1 + x2           % arithmetic, variable 2nd operand: + - * /
x3 = x1 + 10            % arithmetic, constant 2nd operand: + - * /

x2 = [x1]                % memory read, register-indirect address
x2 = [5]                  % memory read, constant address
[x1] = x2                  % memory write, register-indirect address
[5] = x2                    % memory write, constant address

Read x1, 0                   % legacy memory read (address is a constant)
Write x1, 5                   % legacy memory write (address is a constant)

print x1                       % Print - writes x1's value (in hex) to
                                %  the shared log file

v1 = v2 + v3                    % vector add, second operand is a vector
v1 = v2 + 10                     % vector add, second operand is a constant
                                  %  (broadcast to all 8 lanes)
v1 = v2 + x5                      % vector add, second operand is an
                                   %  integer register (broadcast)
% + also works with - and *, in all three forms above

v1 = [x2]                          % vector memory read, register-indirect
                                    %  starting address
v1 = [32]                           % vector memory read, constant
                                     %  starting address
[x2] = v1                            % vector memory write, register-
                                      %  indirect starting address
[32] = v1                             % vector memory write, constant
                                       %  starting address

.loopname                      % label definition (alphanumeric only,
                                %  must start at column 0)
BEQ .loopname                   % conditional branch (see suffix table)
BAL .loopname                    % unconditional branch

% this whole line is a comment and is ignored
x1 = 0    % so is everything after a % on a code line
```

Blank/whitespace-only lines and comments are ignored. Any line that doesn't
match one of the supported patterns, or uses a variable index / constant
value outside 0–255, is treated as invalid and stops compilation with an
error message (the affected task is simply not started — see "Building
and running" below). An out-of-range branch offset (further than ±128
instructions) is also rejected. All of this is unchanged from Lab 4 —
paging affects only what happens to a compiled program's bytes once
they're loaded, not the language or compiler.

### Branch condition suffixes

| Suffix | Meaning | Flag condition |
|---|---|---|
| EQ | equal | Z set |
| NE | not equal | Z clear |
| CS | unsigned higher or same | C set |
| CC | unsigned lower | C clear |
| MI | negative | N set |
| PL | positive or zero | N clear |
| VS | overflow | V set |
| VC | no overflow | V clear |
| HI | unsigned higher | C set and Z clear |
| LS | unsigned lower or same | C clear or Z set |
| GE | greater or equal | N equals V |
| LT | less than | N not equal to V |
| GT | greater than | Z clear and N equals V |
| LE | less than or equal | Z set or N not equal to V |
| AL | always | (ignored) |

### Opcode map (see `opcodes.h`)

| Operation | Variable 2nd operand | Constant 2nd operand |
|---|---|---|
| Add | 0x01 | 0x09 |
| Subtract | 0x02 | 0x0A |
| Multiply | 0x03 | 0x0B |
| Divide | 0x04 | 0x0C |
| Memory read | 0x05 | 0x0D |
| Memory write | 0x06 | 0x0E |
| Data movement | 0x07 | — |
| Print | 0x08 | — |
| Halt | 0x00 | — |
| Branch | 0x10 + condition code (0–14, see table above) | — |
| Vector add | 0x21 (vector) / 0x31 (register) | 0x29 |
| Vector subtract | 0x22 (vector) / 0x32 (register) | 0x2A |
| Vector multiply | 0x23 (vector) / 0x33 (register) | 0x2B |
| Vector memory read | 0x26 | 0x2E |
| Vector memory write | 0x25 | 0x2C |

For Print, dest and operand 1 are always `0`; the register to print is
encoded in operand 2. The opcode map is unchanged from Lab 4 — paging
introduces no new opcodes.

## Memory layout at a glance

With the default constants in `common.h` (`PAGESIZE 512`,
`INSTR_MEM_SIZE 256`, `DATA_MEM_SIZE 4096`, `NP 4`):

| Constant | Value | Meaning |
|---|---|---|
| `NUM_INSTR_PAGES` | 1 | `ceil(256 / 512)` |
| `NUM_DATA_PAGES` | 8 | `4096 / 512` |
| `NUM_LOGICAL_PAGES` | 9 | pages in one process's page table |
| `NUM_PHYSICAL_PAGES` | 37 | `4 * 9 + 1` — frames in physical memory |
| `MEMSIZE` | 18944 bytes | `37 * 512` — size of `memory[]` |

Frame `0` is reserved and never allocated. Each running process's page
table lays out logical page `0` as its instruction page, and logical
pages `1`–`8` as its 8 data pages, in order.

## Building

From the project directory, simply run:

```bash
make
```

This compiles `main.c`, `compiler.c`, `processor.c`, `memory.c`, and
`os.c` into object files and links them into a single executable named
`simulator`.

To rebuild from scratch:

```bash
make clean
make
```

## Running

Unlike Lab 2, the simulator no longer takes a program filename as a
command-line argument. Instead, launch it with no arguments:

```bash
./simulator
```

It prints a `$` prompt and waits for you to type program filenames, one
per line:

```
$ tests/program1.txt data/data1.byte
Loaded pid 1 ('prog1.txt') onto processor 0
$ tests/program2.txt data/data2.byte
Loaded pid 2 ('prog2.txt') onto processor 1
$ exit
```

Each program you name is compiled, has its instruction and data pages
mapped into fresh physical frames, and is either started immediately on
a free processor or queued to run as soon as one frees up (there are `NP`
processors, 4 by default). Typing `exit` stops the shell from accepting
further filenames, but every task already loaded keeps running to
completion; the simulator exits once everything has finished.

### Supplying a data file

To give a program an initial data file, name it explicitly as a second, space-separated token on the same shell line:

```
$ prog1.txt prog1_input.byte
```

If you give just the program name with no second token, that task simply
starts with data memory zeroed out. If you name a data file that doesn't
exist, the shell warns you and still starts the task with zeroed data
memory, rather than silently failing.

> **Note:** when a task finishes, its data file (if it was given one) is
> overwritten with that task's final memory contents, reassembled page by
> page from wherever those pages ended up in physical memory. If you're
> testing against a specific seed input (e.g. an array with known
> values), keep the seed as a separately named file and regenerate it
> before each run, otherwise your original input values are lost after
> the first execution.

### Output

Every `print x<n>` instruction a program executes writes one line to a shared `log.txt` in the current directory (all processors write to the same file, in append mode), in the form:

```
Process id: <proc_id> x<n> : <value in hex>
```

`log.txt` is not cleared automatically between runs — delete it yourself if you want to see only the current run's output.

### Errors you may see

- `ERROR: out of physical memory frames` — every frame is currently
  allocated to a running task. Shouldn't happen under normal use (see
  "Memory layout at a glance" above for why `MEMSIZE` is sized the way it
  is), but can if you push `NP` or the number of concurrently loaded
  tasks past what physical memory was sized for.
- `ERROR: logical address <n> out of range for proc <p>` — a program
  tried to access an address outside its logical instruction/data space
  entirely (likely a bug in the program itself, e.g. a bad computed
  address).
- `ERROR: page fault - unmapped page <n> for proc <p>` — a program
  accessed a logical page that was never allocated for it. Under the
  current design this should only happen from a genuine bug (every page
  a process is entitled to is mapped in full at load time), not from
  normal execution.

## Cleaning up

```bash
make clean
```

Removes all object files, the `simulator` executable, every `tests/*.byte` file, and `log.txt`. It does **not** remove any data files.