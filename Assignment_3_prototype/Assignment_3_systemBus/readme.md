# Mini-Computer Simulator — CS527 Lab 2

## Overview

This project simulates a small computer system made up of three parts:

- **Compiler** (`compiler.c` / `compiler.h`) : reads a program written in a
  simple custom language (variables `x0`–`x255`, arithmetic `+ - * /` with
  either a variable or constant second operand, memory read/write via
  register-indirect or constant addressing, constant assignment, labels, and
  conditional/unconditional branches) and translates it into a 4-byte-per-
  instruction bytecode file, `program.byte`.
- **Memory** (`memory.c` / `memory.h`) : provides `Instruction[256]` and
  `Data[256]` byte arrays. `initialize()` loads `program.byte` into
  instruction memory and `data.byte` into data memory at startup;
  `finalize()` writes the final contents of data memory back out to
  `data.byte` when the program finishes.
- **Processor** (`processor.c` / `processor.h`) : a fetch-decode-execute loop
  over 256 integer registers (`Register[256]`) and four condition flags
  (`Z N C V`). It fetches one 4-byte instruction at a time from instruction
  memory, executes it (arithmetic, memory read/write, constant load, or
  branch), updates the flags after every add/subtract, and halts when it
  hits an opcode of `0`.
- **Opcodes** (`opcodes.h`) : a single shared header defining every opcode
  constant, included by both `compiler.c` and `processor.c` so the two can
  never drift out of sync with each other.

`main.c` wires all four pieces together: compile the input program, load
memory, reset the processor, run the fetch/decode/execute loop until halt,
then write out the final data memory.

## What's new in Lab 2

Lab 2 extends Lab 1 with:

- **Comments** — anything after a `%` on a line is ignored by the compiler.
- **Labels** — a line whose *first* character is `.` (no leading space/tab)
  defines a label, e.g. `.loopback`. Label names are alphanumeric only (no
  underscores or other symbols) and must be the only thing on the line.
- **Branch instructions** — `B<suffix> .label` jumps to a label based on the
  current condition flags (see table below). Branch offsets are encoded as
  a signed relative offset (in instructions, not bytes) from the branch
  instruction itself.
- **Constant-operand arithmetic and memory ops** — every arithmetic and
  memory operation now has two forms: one where the second operand /
  address is a register, and one where it's an immediate constant (see the
  opcode table below).
- **Condition flags** — `Z N C V` are updated after every add/subtract
  (never by multiply, divide, or memory ops) and drive branch decisions.
- `Read`/`Write` are now **legacy** syntax — still compiled correctly, but
  superseded by the bracket-based memory syntax below.

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
error message. An out-of-range branch offset (further than ±128
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
| Halt | 0x00 | — |
| Branch | 0x10 + condition code (0–14, see table above) | — |

## Building

From the project directory, simply run:

```bash
make
```

This compiles `main.c`, `compiler.c`, `processor.c`, and `memory.c` into
object files and links them into a single executable named `simulator`.

To rebuild from scratch :

```bash
make clean
make
```

## Running

The simulator takes the path to your program's source file as a
command-line argument:

```bash
./simulator tests/program.txt
```

This will:
1. Compile `program.txt` into `program.byte`.
2. Load `program.byte` into instruction memory, and `data.byte` (if
   present in the working directory) into data memory.
3. Run the program until it halts.
4. Write the final data memory contents back out to `data.byte`.

If no input file is given, or the given file doesn't exist, the simulator
prints an error and exits without running.

> **Note:** `finalize()` overwrites `data.byte` with the program's final
> memory state every run. If you're testing against a specific seed input
> (e.g. an array with known values), keep the seed as a separately named
> file and copy it into `data.byte` before each run — otherwise your
> original input values are lost after the first execution.

## Cleaning up

```bash
make clean
```

Removes all object files, the `simulator` executable, `program.byte`, and
`data.byte`.
