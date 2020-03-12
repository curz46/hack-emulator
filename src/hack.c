#include "hack.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define VARIABLE_MEMORY 1024

#define C_BITS_EQ(i, c1, c2, c3, c4, c5, c6) ( \
    ((i >> 6 ) & 0x1) == c6 && \
    ((i >> 7 ) & 0x1) == c5 && \
    ((i >> 8 ) & 0x1) == c4 && \
    ((i >> 9 ) & 0x1) == c3 && \
    ((i >> 10) & 0x1) == c2 && \
    ((i >> 11) & 0x1) == c1 )

#define DEST_EQ(i, d1, d2, d3) ( \
    ((i >> 3) & 0x1) == d3 && \
    ((i >> 4) & 0x1) == d2 && \
    ((i >> 5) & 0x1) == d1 )
 
#define JUMP_EQ(i, j1, j2, j3) ( \
    ((i     ) & 0x1) == j3 && \
    ((i >> 1) & 0x1) == j2 && \
    ((i >> 2) & 0x1) == j1 )

#define DEBUG_LEVEL 1

#define DEBUG(level, args...) DEBUG##level(args)

#if DEBUG_LEVEL >= 3
    #define DEBUG3(args...) printf(args)
#else
    #define DEBUG3(args...)
#endif
#if DEBUG_LEVEL >= 2
    #define DEBUG2(args...) printf(args)
#else
    #define DEBUG2(args...)
#endif
#if DEBUG_LEVEL >= 1
    #define DEBUG1(args...) printf(args)
#else
    #define DEBUG1(args...)
#endif

typedef int16_t word_t;

typedef struct program_t {
    // 16-bit registers
    word_t A;
    word_t D;
    word_t memory[VARIABLE_MEMORY];
    word_t* instructions;
    int length;
} program_t;

int get_type(word_t instruction) {
    return instruction >> 15;
}

int evaluate_comp(program_t* program, word_t inst, word_t* out) {
    int a = (inst >> 12) & 0x1;
    if (C_BITS_EQ(inst,1,0,1,0,1,0)) {
        if (a==1) return -1;
        DEBUG(3, "c out=0\n");
        *out = 0;
    } else if (C_BITS_EQ(inst,1,1,1,1,1,1)) {
        if (a==1) return -1;
        DEBUG(3, "c out=1\n");
        *out = 1;
    } else if (C_BITS_EQ(inst,1,1,1,0,1,0)) {
        if (a==1) return -1;
        DEBUG(3, "c out=-1\n");
        *out = -1;
    } else if (C_BITS_EQ(inst,0,0,1,1,0,0)) {
        if (a==1) return -1;
        DEBUG(3, "c out=D\n");
        *out = program->D;
    } else if (C_BITS_EQ(inst,1,1,0,0,0,0)) {
        if (a==0) { *out = program->A; DEBUG(3, "c out=A\n"); }
        else      { *out = program->memory[program->A]; DEBUG(3, "c out=M\n"); }
    } else if (C_BITS_EQ(inst,0,0,1,1,0,1)) {
        if (a==1) return -1;
        DEBUG(3, "c out=~D\n");
        *out = ~program->D;
    } else if (C_BITS_EQ(inst,1,1,0,0,0,1)) {
        if (a==0) { *out = ~program->A; DEBUG(3, "c out=~A\n"); }
        else      { *out = ~program->memory[program->A]; DEBUG(3, "c out=~M\n"); }
    } else if (C_BITS_EQ(inst,0,0,1,1,1,1)) {
        if (a==1) return -1;
        DEBUG(3, "c out=-D\n");
        *out = -program->D;
    } else if (C_BITS_EQ(inst,1,1,0,0,1,1)) {
        if (a==0) { *out = -program->A; DEBUG(3, "c out=-A\n"); }
        else      { *out = -program->memory[program->A]; DEBUG(3, "c out=-M\n"); }
    } else if (C_BITS_EQ(inst,0,1,1,1,1,1)) {
        if (a==1) return -1;
        DEBUG(3, "c out=D+1\n");
        *out = program->D + 1;
    } else if (C_BITS_EQ(inst,1,1,0,1,1,1)) {
        if (a==0) { *out = program->A + 1; DEBUG(3, "c out=A+1\n"); }
        else      { *out = program->memory[program->A] + 1; DEBUG(3, "c out=M+1\n"); } 
    } else if (C_BITS_EQ(inst,0,0,1,1,1,0)) {
        if (a==1) return -1;
        DEBUG(3, "c out=D-1\n");
        *out = program->D - 1;
    } else if (C_BITS_EQ(inst,1,1,0,0,1,0)) {
        if (a==0) { *out = program->A - 1; DEBUG(3, "c out=A-1\n"); }
        else      { *out = program->memory[program->A] - 1; DEBUG(3, "c out=M-1\n"); }
    } else if (C_BITS_EQ(inst,0,0,0,0,1,0)) {
        if (a==0) { *out = program->D + program->A; DEBUG(3, "c out=D+A\n"); }
        else      { *out = program->D + program->memory[program->A]; DEBUG(3, "c out=D+M\n"); }
    } else if (C_BITS_EQ(inst,0,1,0,0,1,1)) {
        if (a==0) { *out = program->D - program->A; DEBUG(3, "c out=D-A\n"); }
        else      { *out = program->D - program->memory[program->A]; DEBUG(3, "c out=D-M\n"); }
    } else if (C_BITS_EQ(inst,0,0,0,1,1,1)) {
        if (a==0) { *out = program->A - program->D; DEBUG(3, "c out=A-D\n"); }
        else      { *out = program->memory[program->A] - program->D; DEBUG(3, "c out=M-D\n"); }
    } else if (C_BITS_EQ(inst,0,0,0,0,0,0)) {
        if (a==0) { *out = program->D & program->A; DEBUG(3, "c out=D & A\n"); }
        else      { *out = program->D & program->memory[program->A]; DEBUG(3, "c out=D & M\n"); }
    } else if (C_BITS_EQ(inst,0,1,0,1,0,1)) {
        if (a==0) { *out = program->D | program->A; DEBUG(3, "c out=D | A\n"); }
        else      { *out = program->D | program->memory[program->A]; DEBUG(3, "c out=D | M\n"); }
    } else {
        DEBUG(3, "c out=<unrecognised instruction>\n");
        return -1;
    }
    return 0;
}

int store_dest(program_t* program, word_t inst, word_t out) {
    if (DEST_EQ(inst,0,0,0)) {
        // do nothing
    } else if (DEST_EQ(inst,0,0,1)) {
        DEBUG(3, "c M=%i\n", out);
        DEBUG(1, "c memory[%i]=%i\n", program->A, out);
        program->memory[program->A] = out;
    } else if (DEST_EQ(inst,0,1,0)) {
        DEBUG(3, "c D=%i\n", out);
        program->D = out;
    } else if (DEST_EQ(inst,0,1,1)) {
        DEBUG(3, "c M,D=%i\n", out);
        DEBUG(1, "c memory[%i]=%i\n", program->A, out);
        program->memory[program->A] = out;
        program->D = out;
    } else if (DEST_EQ(inst,1,0,0)) {
        DEBUG(3, "c A=%i\n", out);
        program->A = out;
    } else if (DEST_EQ(inst,1,1,0)) {
        DEBUG(3, "c A,D=%i\n", out);
        program->A = out;
        program->D = out;
    } else if (DEST_EQ(inst,1,1,1)) {
        DEBUG(3, "c M,D,A=%i\n", out);
        DEBUG(1, "c memory[%i]=%i\n", program->A, out);
        program->memory[program->A] = out;
        program->A = out;
        program->D = out;
    } else {
        // bad instruction
        return -1;
    }
    return 0;
}

// -1 = bad instruction, 0 = don't jump, 1 = jump
int should_jump(program_t* program, word_t inst, word_t out) {
    //printf("jump bits = %i,%i,%i\n", (inst >> 2) & 0x1, (inst >> 1) & 0x1, inst & 0x1);
    if (JUMP_EQ(inst,0,0,0)) {
        DEBUG(3, " JMP 0\n");
        return 0;
    } else if (JUMP_EQ(inst,0,0,1)) {
        DEBUG(3, "c JMP %i > 0\n");
        return out > 0;
    } else if (JUMP_EQ(inst,0,1,0)) {
        DEBUG(3, "c JMP %i == 0\n", out);
        return out == 0;
    } else if (JUMP_EQ(inst,0,1,1)) {
        DEBUG(3, "c JMP %i >= 0\n", out);
        return out >= 0;
    } else if (JUMP_EQ(inst,1,0,0)) {
        DEBUG(3, "c JMP %i < 0\n", out);
        return out < 0;
    } else if (JUMP_EQ(inst,1,0,1)) {
        DEBUG(3, "c JMP %i != 0\n", out);
        return out != 0;
    } else if (JUMP_EQ(inst,1,1,0)) {
        DEBUG(3, "c JMP %i <= 0\n", out);
        return out <= 0;
    } else if (JUMP_EQ(inst,1,1,1)) {
        DEBUG(3, "c JMP 1\n");
        return 1;
    } else {
        return -1;
    }
}

int evaluate_program(char** lines, int length) {
    program_t program;
    // zero-out memory
    for (int i = 0; i < VARIABLE_MEMORY; i++) {
        program.memory[i] = 0;
    }
    program.instructions = (word_t*) malloc(sizeof(word_t) * length);
    program.length       = length;
    for (int i = 0; i < length; i++) {
        program.instructions[i] = strtol(lines[i], NULL, 2);
    }
    DEBUG(1, "Loaded %i instructions.\n", program.length);

    // assume execute at n=0
    word_t n = 0; // current line
    while (1) {
        if (n >= program.length) {
            DEBUG(1, "ERROR: tried to execute out of program bounds");
            return -1;
        }

        DEBUG(2, "n=%i A=%i M=%i D=%i\n", n, program.A, program.memory[program.A], program.D);
        word_t inst = program.instructions[n];

        if (get_type(inst) == 0) { // A-type
            word_t address = inst & 0x7FFF;
            DEBUG(3, "a A=%i\n", address);
            program.A = address;   // store bits to A
            n++;                   // increment line
        } else { // C-type
            word_t out;
            int result;
            result = evaluate_comp(&program, inst, &out);
            if (result == -1) {
                DEBUG(1, "ERROR: bad instruction: %i\n", inst);
                return -1; 
            }
            result = store_dest(&program, inst, out);
            if (result == -1) {
                DEBUG(1, "ERROR: bad instruction: %i\n", inst);
                return -1;
            }
            result = should_jump(&program, inst, out);
            if (result == -1) {
                DEBUG(1, "ERROR: bad instruction: %i\n", inst);
                return -1;
            }
            if (result == 1) { // should jump
                n = program.A;
            } else {
                n++;
            }
        }
    }

    return 0;
}
