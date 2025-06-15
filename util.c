#include "util.h"
#include "pre_prossecor.h"
#include "table.h"

/* Converts a binary string to a hexadecimal string.
 * Pads the binary string with leading zeros to be divisible by 4.
 * Returns a dynamically allocated hex string (must be freed by caller).
 */
char* binary_to_hexa(const char *binary) {
    int len, i, j, value, hex_len, pad_len;
    char *hex, *padded;
    if (!binary) return NULL;
    len = strlen(binary);
    pad_len = (4 - (len % 4)) % 4;
    hex_len = (len + pad_len) / 4;
    padded = (char *)malloc(len + pad_len + 1);
    if (!padded) return NULL;
    for (i = 0; i < pad_len; i++) padded[i] = '0';
    for (i = 0; i < len; i++) padded[pad_len + i] = binary[i];
    padded[len + pad_len] = '\0';
    hex = (char *)malloc(hex_len + 1);
    if (!hex) {
        free(padded);
        return NULL;
    }
    for (i = 0; i < hex_len; i++) {
        value = 0;
        for (j = 0; j < 4; j++) {
            value = value * 2 + (padded[i * 4 + j] - '0');
        }
        hex[i] = (value < 10) ? ('0' + value) : ('A' + value - 10);
    }
    hex[hex_len] = '\0';
    free(padded);
    return hex;
}


/* Checks if a line is empty or a comment (starts with ;) */
int is_comment_or_empty_line(const char *line) {
    while (*line && isspace(*line)) {
        line++;
    }
    return (*line == '\0' || *line == ';');
}

/* Returns 1 if line exceeds allowed maximum length */
int is_line_to_long(const char *line) {
    return strlen(line) > MAX_LINE_LEN;
}

/* Returns 1 if character is a digit (0-9) */
int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

/* Checks if a string is a valid integer (with optional +/- sign) */
int is_valid_integer(const char *str) {
    if (*str == '\0') {
        return 0;
    }
    if (*str == '-' || *str == '+') {
        str++;
    }
    while (*str) {
        if (!is_digit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}

/* Converts an integer to its two's complement binary string representation.
 * Assumes binary has enough space to hold 'bits' characters + '\0'.
 */
void int_to_twos_complement_binary(int value, char *binary, int bits) {
    int i;
    if (value < 0) {
        value = (1 << bits) + value;
    }
    for (i = bits - 1; i >= 0; i--) {
        binary[i] = (value % 2) + '0';
        value /= 2;
    }
    binary[bits] = '\0';
}

/* Returns 1 if the operand is a valid register (r0–r7) */
int is_register(const char *operand) {
    return operand[0] == 'r' && operand[1] >= '0' && operand[1] <= '7' && operand[2] == '\0';
}

/* Determines addressing mode of an operand */
AddressingMode get_addressing_mode(const char *operand) {
    if (operand[0] == '#') return IMMEDIATE;
    if (operand[0] == '&') return RELATIVE;
    if (is_register(operand)) return REGISTER_DIRECT;
    return DIRECT;
}

/* Extracts register number (0–7) from operand r0–r7 */
int get_register_code(const char *operand) {
    if (is_register(operand)) {
        return operand[1] - '0';
    }
    return 0;
}
