#include "pre_prossecor.h"
#include "first_pass.h"
#include "util.h"
#include "table.h"

/* Instruction table: maps instruction mnemonics to their opcode and function code */
Instruction instruction_table[] = {
    {0, 0, "mov"}, {1, 0, "cmp"}, {2, 1, "add"}, {2, 2, "sub"},
    {4, 0, "lea"}, {5, 1, "clr"}, {5, 2, "not"}, {5, 3, "inc"}, {5, 4, "dec"},
    {9, 1, "jmp"}, {9, 2, "bne"}, {9, 3, "jsr"}, {12, 0, "red"},
    {13, 0, "prn"}, {14, 0, "rts"}, {15, 0, "stop"}
};

/* Instruction info table: defines how many operands each instruction expects,
   and which addressing modes are allowed for source and destination operands */
InstructionInfo instruction_info_table[] = {
    {0, 2, {0, 1, 3, -1}, {1, 3, -1}},    /* mov */
    {1, 2, {0, 1, 3, -1}, {0, 1, 3, -1}}, /* cmp */
    {2, 2, {0, 1, 3, -1}, {1, 3, -1}},    /* add */
    {2, 2, {0, 1, 3, -1}, {1, 3, -1}},    /* sub */
    {4, 2, {1, -1}, {1, 3, -1}},          /* lea */
    {5, 1, {-1}, {1, 3, -1}},             /* clr */
    {5, 1, {-1}, {1, 3, -1}},             /* not */
    {5, 1, {-1}, {1, 3, -1}},             /* inc */
    {5, 1, {-1}, {1, 3, -1}},             /* dec */
    {9, 1, {-1}, {1, 2, -1}},             /* jmp */
    {9, 1, {-1}, {1, 2, -1}},             /* bne */
    {9, 1, {-1}, {1, 2, -1}},             /* jsr */
    {12, 1, {-1}, {1, 3, -1}},            /* red */
    {13, 1, {-1}, {0, 1, 3, -1}},         /* prn */
    {14, 0, {-1}, {-1}},                  /* rts */
    {15, 0, {-1}, {-1}}                   /* stop */
};

/* Check if a given addressing mode is allowed by the current instruction */
int is_mode_allowed(int mode, int *allowed_modes) {
    int i;
    for (i = 0; allowed_modes[i] != -1; i++) {
        if (mode == allowed_modes[i]) {
            return 1;
        }
    }
    return 0;
}

/* Handle the .data directive:
   If there is a label, store it.
   Then parse each number and store it in memory and update symbol if needed. */
void handle_data_directive(const char *label, char *token, int *address, int *DC) {
    int symbol_index;
    int value;

    if (label) {
        add_symbol(label, *address);
        symbol_index = get_symbol_count() - 1;
    } else {
        symbol_index = -1;
    }

    while ((token = strtok(NULL, " ,\t\n"))) {
        if (is_valid_integer(token)) {
            value = atoi(token);
            add_object((*address)++, value);
            if (symbol_index != -1) {
                add_data_to_symbol(symbol_index, value);
            }
            (*DC)++;
        } else {
            fprintf(stderr, "Error: Invalid Integer in .data: %s\n", token);
        }
    }
}

/* Handle the .string directive:
   If label is present, save it.
   Then add each character to memory followed by null terminator. */
void handle_string_directive(const char *label, char *token, int *address, int *DC) {
    int symbol_index;
    int i;

    if (label) {
        add_symbol(label, *address);
        symbol_index = get_symbol_count() - 1;
    } else {
        symbol_index = -1;
    }

    token = strtok(NULL, "\t\n");
    if (!token) {
        fprintf(stderr, "Error: Missing string after .string\n");
        return;
    }

    if (token[0] == '"' && token[strlen(token) - 1] == '"') {
        for (i = 1; i < (int)strlen(token) - 1; i++) {
            add_object((*address)++, token[i]);
            if (symbol_index != -1) {
                add_data_to_symbol(symbol_index, token[i]);
            }
            (*DC)++;
        }
        add_object((*address)++, '\0');
        if (symbol_index != -1) {
            add_data_to_symbol(symbol_index, '\0');
        }
        (*DC)++;
    } else {
        fprintf(stderr, "Error: Invalid string format in .string: %s\n", token);
    }
}

/* First pass on the source file:
   Reads each line, detects labels and directives,
   and processes them or sends instructions for further handling. */
void first_pass(const char *file_name, int *IC, int *DC) {
    FILE *file;
    char line[MAX_LINE_LEN];
    char line_copy[MAX_LINE_LEN];
    int address;
    char *token;

    file = fopen(file_name, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    address = MEMORY_START;

    while (fgets(line, sizeof(line), file)) {
        if (is_comment_or_empty_line(line)) {
            continue;
        }

        if (is_line_to_long(line)) {
            fprintf(stderr, "Error: Line exceeds maximum length of %d\n", MAX_LINE_LEN);
            continue;
        }

        strcpy(line_copy, line);
        token = strtok(line_copy, " \t\n");
        if (!token) {
            continue;
        }

        /* Check for label */
        if (strchr(token, ':')) {
            char *label = token;
            label[strlen(label) - 1] = '\0';
            add_symbol(label, address);
            token = strtok(NULL, " \t\n");
        }

        if (token && !strcmp(token, ".data")) {
            handle_data_directive(NULL, token, &address, DC);
        } else if (token && !strcmp(token, ".string")) {
            handle_string_directive(NULL, token, &address, DC);
        } else if (token && !strcmp(token, ".entry")) {
            token = strtok(NULL, " \t\n");
            if (token) {
                add_entry(token, -1);
            }
        } else if (token && !strcmp(token, ".extern")) {
            token = strtok(NULL, " \t\n");
            if (token) {
                add_extern(token, -1);
            }
        } else if (token) {
            handle_instruction(token, &address, IC);
        }
    }

    fclose(file);
}

/* Handle an instruction line:
   - Check if instruction is valid
   - Parse operands and validate their addressing modes
   - Encode the instruction into binary and add it to memory
   - Add extra words for operands as needed (with placeholders for labels) */
void handle_instruction(char *instruction, int *address, int *instruction_counter) {
    int i;
    int opcode = -1;
    int funct = 0;
    AddressingMode source_mode = -1;
    AddressingMode destination_mode = -1;
    int source_register = 0;
    int destination_register = 0;
    int operand_count = 0;
    char *operand1 = NULL;
    char *operand2 = NULL;
    CodeWord code_word;
    unsigned int encoded_value = 0;
    int is_valid = 0;

    /* Step 1: Find the instruction's opcode and funct from the table */
    for (i = 0; i < (int)(sizeof(instruction_table) / sizeof(Instruction)); i++) {
        if (strcmp(instruction, instruction_table[i].name) == 0) {
            opcode = instruction_table[i].opcode;
            funct = instruction_table[i].funct;
            break;
        }
    }

    /* If the instruction is unknown, print error and return */
    if (opcode == -1) {
        fprintf(stderr, "Error: Unknown instruction '%s'\n", instruction);
        return;
    }

    /* Step 2: Extract up to two operands (comma-separated) */
    operand1 = strtok(NULL, ", \t\n");
    operand2 = strtok(NULL, ", \t\n");

    if (operand1 && operand2) {
        operand_count = 2;
    } else if (operand1) {
        operand_count = 1;
    } else {
        operand_count = 0;
    }

    /* Step 3: Validate number of operands and addressing modes */
    for (i = 0; i < (int)(sizeof(instruction_info_table) / sizeof(InstructionInfo)); i++) {
        if (instruction_info_table[i].opcode == opcode) {

            /* Check operand count */
            if (instruction_info_table[i].num_operands != operand_count) {
                fprintf(stderr,
                        "Error: Instruction '%s' expects %d operand(s), got %d\n",
                        instruction, instruction_info_table[i].num_operands, operand_count);
                return;
            }

            /* If two operands, check both */
            if (operand_count == 2) {
                source_mode = get_addressing_mode(operand1);
                destination_mode = get_addressing_mode(operand2);

                if (!is_mode_allowed(source_mode, instruction_info_table[i].legal_src_modes)) {
                    fprintf(stderr,
                            "Error: Illegal source operand addressing mode in instruction '%s'\n",
                            instruction);
                    return;
                }
                if (!is_mode_allowed(destination_mode, instruction_info_table[i].legal_dst_modes)) {
                    fprintf(stderr,
                            "Error: Illegal destination operand addressing mode in instruction '%s'\n",
                            instruction);
                    return;
                }

                /* Get register codes if mode is REGISTER_DIRECT */
                if (source_mode == REGISTER_DIRECT) {
                    source_register = get_register_code(operand1);
                    if (source_register == -1) {
                        fprintf(stderr, "Error: Invalid source register '%s'\n", operand1);
                        return;
                    }
                }

                if (destination_mode == REGISTER_DIRECT) {
                    destination_register = get_register_code(operand2);
                    if (destination_register == -1) {
                        fprintf(stderr, "Error: Invalid destination register '%s'\n", operand2);
                        return;
                    }
                }
            }

            /* If one operand, check destination only */
            else if (operand_count == 1) {
                destination_mode = get_addressing_mode(operand1);
                if (!is_mode_allowed(destination_mode, instruction_info_table[i].legal_dst_modes)) {
                    fprintf(stderr,
                            "Error: Illegal operand addressing mode in instruction '%s'\n",
                            instruction);
                    return;
                }

                if (destination_mode == REGISTER_DIRECT) {
                    destination_register = get_register_code(operand1);
                    if (destination_register == -1) {
                        fprintf(stderr, "Error: Invalid register '%s'\n", operand1);
                        return;
                    }
                }
            }

            is_valid = 1;
            break;
        }
    }

    /* Step 4: If instruction is invalid after checking rules, return */
    if (!is_valid) {
        return;
    }

    /* Step 5: Build the CodeWord struct with all encoded fields */
    code_word.opcode = opcode;
    code_word.funct = funct;
    code_word.src_addr = (operand_count == 2) ? source_mode : 0;
    code_word.src_reg = (operand_count == 2) ? source_register : 0;
    code_word.dest_addr = destination_mode;
    code_word.dest_reg = destination_register;
    code_word.A = 1;
    code_word.R = 0;
    code_word.E = 0;
    code_word.unused = 0;

    /* Step 6: Encode the instruction into a 24-bit binary word */
    encoded_value = 0;
    encoded_value |= (code_word.unused & 0x7) << 21;
    encoded_value |= (code_word.opcode & 0xF) << 17;
    encoded_value |= (code_word.src_reg & 0x7) << 14;
    encoded_value |= (code_word.src_addr & 0x3) << 12;
    encoded_value |= (code_word.dest_reg & 0x7) << 9;
    encoded_value |= (code_word.dest_addr & 0x3) << 7;
    encoded_value |= (code_word.funct & 0xF) << 3;
    encoded_value |= (code_word.A & 0x1) << 2;
    encoded_value |= (code_word.R & 0x1) << 1;
    encoded_value |= (code_word.E & 0x1);

    add_object(*instruction_counter, encoded_value);
    (*instruction_counter)++;
    (*address)++;

    /* Step 7: Add extra memory words for non-register operands */

    /* Case: Two operands */
    if (operand_count == 2) {

        /* Source operand */
        if (source_mode != REGISTER_DIRECT) {
            if (source_mode == IMMEDIATE) {
                int value;
                value = atoi(operand1 + 1);
                if (value < 0) {
                    value = (1 << 21) + value;
                }
                add_object(*instruction_counter, ((value & 0x1FFFFF) << 3) | ABSULUTE);
            } else {
                add_object(*instruction_counter, 0);
                add_pending_word(operand1, *instruction_counter, source_mode);
            }
            (*instruction_counter)++;
            (*address)++;
        }

        /* Destination operand */
        if (destination_mode != REGISTER_DIRECT) {
            if (destination_mode == IMMEDIATE) {
                int value;
                value = atoi(operand2 + 1);
                if (value < 0) {
                    value = (1 << 21) + value;
                }
                add_object(*instruction_counter, ((value & 0x1FFFFF) << 3) | ABSULUTE);
            } else {
                add_object(*instruction_counter, 0);
                add_pending_word(operand2, *instruction_counter, destination_mode);
            }
            (*instruction_counter)++;
            (*address)++;
        }

    }

    /* Case: One operand */
    else if (operand_count == 1) {
        if (destination_mode != REGISTER_DIRECT) {
            if (destination_mode == IMMEDIATE) {
                int value;
                value = atoi(operand1 + 1);
                if (value < 0) {
                    value = (1 << 21) + value;
                }
                add_object(*instruction_counter, ((value & 0x1FFFFF) << 3) | ABSULUTE);
            } else {
                add_object(*instruction_counter, 0);
                add_pending_word(operand1, *instruction_counter, destination_mode);
            }
            (*instruction_counter)++;
            (*address)++;
        }
    }
}
