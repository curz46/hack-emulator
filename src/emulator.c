#include <stdlib.h>

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

typedef short int word_t;

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
    int a = (inst >> 6) & 0x07;
    if (C_BITS_EQ(inst,1,0,1,0,1,0)) {
        if (a==1) return -1;
        *out = 0;
    } else if (C_BITS_EQ(inst,1,1,1,1,1,1)) {
        if (a==1) return -1;
        *out = 1;
    } else if (C_BITS_EQ(inst,1,1,1,0,1,0)) {
        if (a==1) return -1;
        *out = -1;
    } else if (C_BITS_EQ(inst,0,0,1,1,0,0)) {
        if (a==1) return -1;
        *out = program->D;
    } else if (C_BITS_EQ(inst,1,1,0,0,0,0)) {
        if (a==0) *out = program->A;
        else      *out = program->memory[program->A];
    } else if (C_BITS_EQ(inst,0,0,1,1,0,1)) {
        if (a==1) return -1;
        *out = ~program->D;
    } else if (C_BITS_EQ(inst,1,1,0,0,0,1)) {
        if (a==0) *out = ~program->A;
        else      *out = ~program->memory[program->A];
    } else if (C_BITS_EQ(inst,0,0,1,1,1,1)) {
        if (a==1) return -1;
        *out = -program->D;
    } else if (C_BITS_EQ(inst,1,1,0,0,1,1)) {
        if (a==0) *out = -program->A;
        else      *out = -program->memory[program->A];
    } else if (C_BITS_EQ(inst,0,1,1,1,1,1)) {
        if (a==1) return -1;
        *out = program->D + 1;
    } else if (C_BITS_EQ(inst,1,1,0,1,1,1)) {
        if (a==0) *out = program->A + 1;
        else      *out = program->memory[program->A] + 1;
    } else if (C_BITS_EQ(inst,0,0,1,1,1,0)) {
        if (a==1) return -1;
        *out = program->D - 1;
    } else if (C_BITS_EQ(inst,1,1,0,0,1,0)) {
        if (a==0) *out = program->A - 1;
        else      *out = program->memory[program->A] - 1;
    } else if (C_BITS_EQ(inst,0,0,0,0,1,0)) {
        if (a==0) *out = program->D + program->A;
        else      *out = program->D + program->memory[program->A];
    } else if (C_BITS_EQ(inst,0,1,0,0,1,1)) {
        if (a==0) *out = program->D - program->A;
        else      *out = program->D - program->memory[program->A];
    } else if (C_BITS_EQ(inst,0,0,0,1,1,1)) {
        if (a==0) *out = program->A - program->D;
        else      *out = program->memory[program->A] - program->D;
    } else if (C_BITS_EQ(inst,0,0,0,0,0,0)) {
        if (a==0) *out = program->D & program->A;
        else      *out = program->D & program->memory[program->A];
    } else if (C_BITS_EQ(inst,0,1,0,1,0,1)) {
        if (a==0) *out = program->D | program->A;
        else      *out = program->D | program->memory[program->A];
    } else {
        return -1;
    }
    return 0;
}

int store_dest(program_t* program, word_t inst, word_t out) {
    if (DEST_EQ(inst,0,0,0)) {
        // do nothing
    } else if (DEST_EQ(inst,0,0,1)) {
        program->memory[program->A] = out;
    } else if (DEST_EQ(inst,0,1,0)) {
        program->D = out;
    } else if (DEST_EQ(inst,0,1,1)) {
        program->memory[program->A] = out;
        program->D = out;
    } else if (DEST_EQ(inst,1,0,0)) {
        program->A = out;
    } else if (DEST_EQ(inst,1,1,0)) {
        program->A = out;
        program->D = out;
    } else if (DEST_EQ(inst,1,1,1)) {
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
    if (JUMP_EQ(inst,0,0,0)) {
        return 0;
    } else if (JUMP_EQ(inst,0,0,1)) {
        return out > 0;
    } else if (JUMP_EQ(inst,0,1,0)) {
        return out == 0;
    } else if (JUMP_EQ(inst,0,1,1)) {
        return out >= 0;
    } else if (JUMP_EQ(inst,1,0,0)) {
        return out < 0;
    } else if (JUMP_EQ(inst,1,0,1)) {
        return out != 0;
    } else if (JUMP_EQ(inst,1,1,0)) {
        return out <= 0;
    } else if (JUMP_EQ(inst,1,1,1)) {
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
        program.instructions[i] = atoi(lines[i]);
    }

    // assume execute at n=0
    int n = 0; // current line
    while (1) {
        if (n >= program.length) return -1; //error: tried to execute out of bounds

        word_t inst = program.instructions[n];
        if (get_type(inst) == 0) { // A-type
            word_t address = inst & 0x7FFF;
            program.A = address;   // store bits to A
            n++;                   // increment line
        } else { // C-type
            word_t out;
            int result;
            result = evaluate_comp(&program, inst, &out);
            if (result == -1) return -1; //error: bad instruction
            result = store_dest(&program, inst, out);
            if (result == -1) return -1; //error: bad instruction
            result = should_jump(&program, inst, out);
            if (result == -1) return -1; //error: bad instruction
            if (result == 1) { // should jump
                n = program.A;
            } else {
                n++;
            }
        }

        printf("n=%i A=%i D=%i\n", n, program.A, program.D);
    }

    return 0;
}
