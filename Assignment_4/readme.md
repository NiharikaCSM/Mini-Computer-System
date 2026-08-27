# Mini-Computer Simulator — CS527 Lab 4

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
- **Memory** (`memory.c` / `memory.h`) : provides `Instruction[NP][256]`
  and `Data[NP][4096]` byte arrays, one instruction and data memory per
  processor, so concurrently running tasks never see each other's state.
  `initialize(proc_id, programFile, dataFile)` loads a task's compiled
  bytecode (and, if given, its data file) into that processor's memory;
  `finalize(proc_id, dataFile)` writes the final contents of that
  processor's data memory back out to the given file when the task halts.
- **Processor** (`processor.c` / `processor.h`) : a fetch-decode-execute
  loop, replicated across `NP` processors. Each processor has its own 256
  integer registers, 32 vector registers (8 lanes of 32 bits each), a
  program counter, and its own set of condition flags (`Z N C V`). It
  fetches one 4-byte instruction at a time, executes it (integer/vector
  arithmetic, memory read/write, constant load, branch, or Print), updates
  the flags after every integer add/subtract, and halts when it hits an
  opcode of `0`.
- **Opcodes** (`opcodes.h`) : a single shared header defining every opcode
  constant, included by both `compiler.c` and `processor.c` so the two can
  never drift out of sync with each other.
- **OS** (`os.c` / `os.h`) : a thin operating-system layer sitting on top
  of the processors, a **shell** that reads program and data
  filenames from the terminal without blocking, a **loader** that compiles
  a program and assigns it to a free processor (or queues it if all are
  busy), and a **scheduler** that round-robins every active task through a
  fixed time slice on its processor, in a loop, until every task has
  finished and the shell has been told to exit.

`main.c` is now just `osInit(); osRun();` — everything that used to happen
once in `main()` (compiling, loading, resetting, running, finalizing)
now happens per-task, driven by the OS, since a program is something the
OS receives from the shell at run time rather than a fixed command-line
argument.

## New Features

- **Vector registers and vector arithmetic** — 32 vector registers `v0`–
  `v31`, each holding 8 lanes of 32-bit integers. Vector add/subtract/
  multiply support a second operand that's another vector, a constant, or
  an integer register (broadcast to all 8 lanes). Vector memory
  read/write moves 8 consecutive 32-bit words to/from data memory, with
  either a register-indirect or constant starting address.
- **`Print` instruction** — `print x<n>` writes one line to a shared log
  file: `Process id: <proc_id> x<n> : <value in hex>`. This is currently
  the *only* thing written to the log, nothing else is logged
  automatically.
- **Multiprocessing** — `NP` processors (4 by default, see `common.h`),
  each with fully independent registers, flags, and memory, so several
  compiled programs can run concurrently without interfering with each
  other.
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
    processor the moment it frees up, until nothing is left to run.
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
instructions) is also rejected.

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
encoded in operand 2.

## Building

From the project directory, simply run:

```bash
make
```

This compiles `main.c`, `compiler.c`, `processor.c`, `memory.c`, and the
new `os.c` into object files and links them into a single executable
named `simulator`.

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

Each program you name is compiled and either started immediately on a
free processor, or queued to run as soon as one frees up (there are `NP`
processors, 4 by default). Typing `exit` stops the shell from accepting
further filenames, but every task already loaded keeps running to
completion; the simulator exits once everything has finished.


### Supplying a data file

To give a program an initial data file, name it explicitly as a second, space-separated token on the same shell line:

```
$ prog1.txt prog1_input.byte
```

If you give just the program name with no second token, that task simply starts with data memory zeroed out. If you name a data file that doesn't exist, the shell warns you and still starts the task with zeroed data memory, rather than silently failing.

> **Note:** when a task finishes, its data file (if it was given one) is
> overwritten with that task's final memory contents. If you're testing
> against a specific seed input (e.g. an array with known values), keep
> the seed as a separately named file and regenerate it before each run,
> otherwise your original input values are lost after the first
> execution.

### Output

Every `print x<n>` instruction a program executes writes one line to a shared `log.txt` in the current directory (all processors write to the same file, in append mode), in the form:

```
Process id: <proc_id> x<n> : <value in hex>
```

`log.txt` is not cleared automatically between runs — delete it yourself if you want to see only the current run's output.

## Cleaning up

```bash
make clean
```

Removes all object files, the `simulator` executable, every `tests/*.byte` file, and `log.txt`. It does **not** remove any data files.