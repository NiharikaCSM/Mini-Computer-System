#include <stdio.h>
#include "compiler.h"
#include "processor.h"
#include "memory.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Please provide parameters in the correct format");
        return 1;
    }

    FILE *test = fopen(argv[1], "r");
    if (!test) {
        printf("Error: input file '%s' not found\n", argv[1]);
        return 1;
    }
    fclose(test);

    compile(argv[1], "program.byte");
    initialize();
    reset();

    while (!end_of_simulation) {
        fetch();
        decode();
        execute();
    }
    printRegisters();
    finalize();

    printf("Simulation complete\n");
    return 0;
}
