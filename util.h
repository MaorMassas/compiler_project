#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*defenition for ARE*/
#define ABSULUTE 4
#define RELOCATABLE 2
#define EXTERNAL 1

typedef enum {
    IMMEDIATE = 0,
    DIRECT = 1,
    RELATIVE = 2,
    REGISTER_DIRECT = 3
} AddressingMode;

/* Bit-field structure for an instruction word (24-bit) */
typedef struct {
    unsigned int E:1;        /* External flag */
    unsigned int R:1;        /* Relocatable flag */
    unsigned int A:1;        /* Absolute flag */
    unsigned int funct:4;    /* Function code */
    unsigned int dest_addr:2;/* Destination addressing mode */
    unsigned int dest_reg:3; /* Destination register */
    unsigned int src_addr:2;  /* Source addressing mode */
    unsigned int src_reg:3;  /* Source register */
    unsigned int opcode:4;   /* Operation code */
    unsigned int unused:3;   /* Unused bits */
} CodeWord;

/* Bit-field structure for a data/additional word (24-bit) */
typedef struct {
    unsigned int E:1;        /* External flag */
    unsigned int R:1;        /* Relocatable flag */
    unsigned int A:1;        /* Absolute flag */
    unsigned int value:21;   /* 21-bit value */
} DataWord;


/*Checks if the line is empty or a comment*/
int is_comment_or_empty_line(const char *line);

/* Checks if a given input line exceeds the maximum allowed length (81 chars including '\0') */
int is_line_to_long(const char *line);

/* Converts a binary string to a hexadecimal string using dynamic allocation */
char* binary_to_hexa(const char *binary);

/* Returns 1 if the character is a digit ('0' to '9'), otherwise 0 */
int is_digit(char c);

/* Checks if a string represents a valid integer (with optional sign) */
int is_valid_integer(const char *str);

/* Converts an integer to its two's complement binary representation with a fixed number of bits */
void int_to_twos_complement_binary(int value, char *binary, int bits);

/* Returns the addressing mode of the given operand (e.g., IMMEDIATE, DIRECT, REGISTER) */
AddressingMode get_addressing_mode(const char *operand);

/* Extracts the register code from an operand string like "r3" or "r0" */
int get_register_code(const char *operand);



#endif /* UTIL_H */
