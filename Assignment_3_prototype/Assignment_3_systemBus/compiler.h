#ifndef COMPILER_H
#define COMPILER_H

/* Reads the human-readable source program from input_filename,
 * translates each line into a 4-byte instruction, and writes the
 * result to output_filename (program.byte). */
void compile(const char *input_filename, const char *output_filename);

#endif
