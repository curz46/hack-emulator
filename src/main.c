#include "util.h"
#include "hack.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("provide a filename\n");
        exit(1);
    }
    char* filename = argv[1];
    FILE* file = fopen(filename, "r");

    int length;
    char** lines = readlines(file, &length);

    evaluate_program(lines, length);

    return 0;
}
