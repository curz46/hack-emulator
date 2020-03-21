#ifndef HACK__GUARD_H
#define HACK__GUARD_H

#include <stdio.h>
#include <stdint.h>

/**
 * The Hack architecture mandates at least 24577 bytes allocated for execution.
 * - 0-1024 is considered "reserved" memory for the VM's execution.
 * - 1024+ is variable memory.
 * - The Hack screen is a black & white display with 512 columns & 256 rows.
 *   Each pixel requires one bit. The base address of the screen is 16384.
 * - The Hack keyboard is a single 16-bit word at address 24576.
 * We allocate up until the last specified address.
 */
#define ALLOCATED_MEMORY 24577

typedef int16_t word_t;

typedef struct program_t {
    // 16-bit registers
    word_t A;
    word_t D;
    word_t memory[ALLOCATED_MEMORY];
    word_t* instructions;
    int length;
} program_t;

/**
 * Read the file provided as a Hack program. The returned program_t struct
 * can be passed to `evaluate_program` in order to execute it.
 */
program_t read_program(FILE* file);

/**
 * Evaluate the provided program. A return value of 0 means the program
 * executed successfully, while anything else implies an error occurred.
 */
int evaluate_program(program_t program);

#endif
