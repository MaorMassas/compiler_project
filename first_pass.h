#ifndef FIRST_PASS_H

#define FIRST_PASS_H

#include "util.h"

typedef struct {
    int opcode;
    int funct;
    char* name;
} Instruction;

/* Define legal addressing modes per instruction */
typedef struct {
    int opcode;
    int num_operands;
    int legal_src_modes[4];
    int legal_dst_modes[4];
} InstructionInfo;

void handle_instruction(char *instruction, int *address, int *IC);
void first_pass(const char *filename, int *IC, int *DC);
int get_opcode(const char *mnemonic);
void handle_operand_word(char *operand, AddressingMode mode, int *IC, int *address);

#endif
