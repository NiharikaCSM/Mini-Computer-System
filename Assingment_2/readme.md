# Mini-Computer Simulator — CS527 Lab 1

## Overview

This project simulates a small computer system made up of three parts:

- **Compiler** (`compiler.c` / `compiler.h`) : reads a program written in a simple custom language (variables `x0`–`x255`, basic math `+ - * /`, memory `Read`/`Write`, and constant assignment `x = value`) and translates it into a 4-byte-per-instruction bytecode file, `program.byte`.
- **Memory** (`memory.c` / `memory.h`) : provides two 256-byte arrays, `Instruction[256]` and `Data[256]`. `initialize()` loads `program.byte` into instruction memory at startup; `finalize()` writes the final contents of data memory out to `data.byte` when the program finishes.
- **Processor** (`processor.c` / `processor.h`) : a simple fetch-decode-execute loop over 256 integer registers (`Register[256]`). It fetches one 4-byte instruction at a time from instruction memory, executes it (arithmetic, memory read/write, or constant
load), and halts when it hits an opcode of `0`.

`main.c` wires all three pieces together: compile the input program, load memory, reset the processor, run the fetch/decode/execute loop until halt, then write out the final data memory.

## Language quick reference

```
x5 = 10            # constant assignment
x3 = x1 + x2       # arithmetic: + - * /
Read x1, 0         # load Data[0] into register x1
Write x1, 5        # store register x1 into Data[5]
```

Blank/whitespace-only lines are ignored. Any line that doesn't match one of the patterns above, or uses a variable index / value outside 0–255, is treated as invalid and stops compilation with an error message.

## Building

From the project directory, simply run:

```bash
make
```

This compiles `main.c`, `compiler.c`, `processor.c`, and `memory.c` into object files and links them into a single executable named `simulator`.

To rebuild from scratch :

```bash
make clean
make
```

## Running

The simulator takes the path to your program's source file as a command-line argument:

```bash
./simulator tests/program.txt
```

This will:
1. Compile `myprogram.txt` into `program.byte`.
2. Load `program.byte` into instruction memory.
3. Run the program until it halts.
4. Write the final data memory contents to `data.byte`.

If no input file is given, or the given file doesn't exist, the simulator prints an error and exits without running.

## Cleaning up

```bash
make clean
```

Removes all object files, the `simulator` executable, `program.byte`, and `data.byte`.