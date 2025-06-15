#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "pre_prossecor.h"
#include "table.h"

/* Global static tables and counters for the assembler's internal data */
static PendingWord *pending_words = NULL;
static int pending_count = 0;
static int symbol_count = 0;
static int entry_count = 0;
static int extern_count = 0;
static int object_count = 0;

static Symbol *symbol_table = NULL;
static Entry *entry_table = NULL;
static Extern *extern_table = NULL;
static Object *object_table = NULL;

/* Frees all dynamic memory allocations used by the assembler */
void free_memory(void) {
    if (entry_table != NULL) {
        free(entry_table);
        entry_table = NULL;
    }
    if (extern_table != NULL) {
        free(extern_table);
        extern_table = NULL;
    }
    if (object_table != NULL) {
        free(object_table);
        object_table = NULL;
    }
    if (symbol_table != NULL) {
        free(symbol_table);
        symbol_table = NULL;
    }
    if (pending_words != NULL) {
        free(pending_words);
        pending_words = NULL;
        pending_count = 0;
    }
}

/* Adds a new entry symbol (.entry directive) */
void add_entry(const char *label, int address) {
    Entry *temp;

    temp = realloc(entry_table, (entry_count + 1) * sizeof(Entry));
    if (!temp) {
        fprintf(stderr, "Failed to allocate memory for entry table\n");
        free_memory();
        exit(EXIT_FAILURE);
    }
    entry_table = temp;

    strncpy(entry_table[entry_count].label, label, MAX_LABEL_LENGTH - 1);
    entry_table[entry_count].label[MAX_LABEL_LENGTH - 1] = '\0';
    entry_table[entry_count].address = address;

    entry_count++;
}

/* Adds a new external label usage (.extern directive or use) */
void add_extern(const char *symbol, int address) {
    Extern *temp;
    int i;

    /* Avoid duplicates */
    for (i = 0; i < extern_count; i++) {
        if (!strcmp(extern_table[i].symbol, symbol) &&
            extern_table[i].address == address) {
            return;
        }
    }

    temp = realloc(extern_table, (extern_count + 1) * sizeof(Extern));
    if (!temp) {
        fprintf(stderr, "Failed to allocate memory for extern table\n");
        free_memory();
        exit(EXIT_FAILURE);
    }
    extern_table = temp;

    strncpy(extern_table[extern_count].symbol, symbol, MAX_LABEL_LENGTH - 1);
    extern_table[extern_count].symbol[MAX_LABEL_LENGTH - 1] = '\0';
    extern_table[extern_count].address = address;

    extern_count++;
}

/* Adds a new symbol (label) to the symbol table */
void add_symbol(const char *label, int address) {
    Symbol *temp;
    int i;

    /* Ignore duplicates */
    for (i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].label, label) == 0) {
            return;
        }
    }

    temp = realloc(symbol_table, (symbol_count + 1) * sizeof(Symbol));
    if (temp == NULL) {
        fprintf(stderr, "Failed to allocate memory for symbol table\n");
        free_memory();
        exit(EXIT_FAILURE);
    }
    symbol_table = temp;

    strncpy(symbol_table[symbol_count].label, label, MAX_LABEL_LENGTH - 1);
    symbol_table[symbol_count].label[MAX_LABEL_LENGTH - 1] = '\0';
    symbol_table[symbol_count].address = address;
    symbol_table[symbol_count].data_count = 0;
    symbol_count++;
}

/* Adds a value to a symbol's associated data (for .data or .string) */
void add_data_to_symbol(int symbol_index, int value) {
    if (symbol_index < symbol_count) {
        if (symbol_table[symbol_index].data_count < SIZE_DATA) {
            symbol_table[symbol_index].data_value[symbol_table[symbol_index].data_count++] = value;
        } else {
            fprintf(stderr, "Error: Exceeded data storage for symbol %s\n", symbol_table[symbol_index].label);
        }
    } else {
        fprintf(stderr, "Error: Invalid symbol index %d\n", symbol_index);
    }
}

/* Adds a word to the object table (machine code memory) */
void add_object(unsigned int address, int value) {
    Object *temp;

    if (address >= MAX_MEMORY) {
        fprintf(stderr, "Error: Exceeded memory limit of %d bytes\n", MAX_MEMORY);
        free_memory();
        exit(EXIT_FAILURE);
    }

    temp = realloc(object_table, (object_count + 1) * sizeof(Object));
    if (temp == NULL) {
        fprintf(stderr, "Failed to allocate memory for object table\n");
        free_memory();
        exit(EXIT_FAILURE);
    }

    object_table = temp;
    object_table[object_count].address = address;
    object_table[object_count].value = value & 0xFFFFFF; /* 24-bit */
    object_count++;
}

/* Determines ARE type (A/R/E) for an operand */
int get_are_code(const char *operand) {
    int i;

    if (operand == NULL || operand[0] == '\0') {
        fprintf(stderr, "Error: Invalid operand passed to get_are_code\n");
        return -1;
    }

    if (operand[0] == '#') {
        return ABSULUTE;
    }

    for (i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].label, operand) == 0) {
            return RELOCATABLE;
        }
    }

    for (i = 0; i < extern_count; i++) {
        if (strcmp(extern_table[i].symbol, operand) == 0) {
            return EXTERNAL;
        }
    }

    return RELOCATABLE;
}

/* Writes the .ob (object) file with IC, DC and all code words */
void write_object_file(const char *filename, int IC, int DC) {
    FILE *file;
    int i;

    file = fopen(filename, "w");
    if (!file) {
        perror("Error opening object file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%d %d\n", IC, DC);
    for (i = 0; i < object_count; i++) {
        fprintf(file, "%04d %06X\n", object_table[i].address, object_table[i].value);
    }
    fclose(file);
}

/* Writes the .ent file with all entry symbols and their addresses */
void write_entries_file(const char *filename) {
    FILE *file;
    int i, j;
    Symbol *symbol_table = get_symbol_table();
    int symbol_count = get_symbol_count();

    file = fopen(filename, "w");
    if (!file) {
        perror("Error opening entries file");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < entry_count; i++) {
        for (j = 0; j < symbol_count; j++) {
            if (strcmp(entry_table[i].label, symbol_table[j].label) == 0) {
                fprintf(file, "%s %04d\n", entry_table[i].label, symbol_table[j].address);
                break;
            }
        }
    }

    fclose(file);
}

/* Writes the .ext file with all used external labels and their usage addresses */
void write_externals_file(const char *filename) {
    FILE *file;
    int i;

    file = fopen(filename, "w");
    if (!file) {
        perror("Error opening externals file");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < extern_count; i++) {
        if (extern_table[i].address < 0) {
            continue;
        }
        fprintf(file, "%s %04d\n", extern_table[i].symbol, extern_table[i].address);
    }

    fclose(file);
}

/* Accessor functions (getters) for each internal table and count */

int get_symbol_count(void) {
    return symbol_count;
}

int get_entry_count(void) {
    return entry_count;
}

int get_extern_count(void) {
    return extern_count;
}

int get_object_count(void) {
    return object_count;
}

Symbol* get_symbol_table(void) {
    return symbol_table;
}

Extern* get_extern_table(void) {
    return extern_table;
}

Entry* get_entry_table(void) {
    return entry_table;
}

Object* get_object_table(void) {
    return object_table;
}

/* Finds and returns the address of a label from the symbol table */
int resolve_direct_address(const char *label) {
    Symbol *symbol_table = get_symbol_table();
    int symbol_count = get_symbol_count();
    int i;

    for (i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].label, label) == 0) {
            return symbol_table[i].address;
        }
    }

    return -1;
}

/* Adds a word to the pending list to be resolved in the second pass */
void add_pending_word(const char *label, int address, AddressingMode mode) {
    PendingWord *temp;

    temp = realloc(pending_words, (pending_count + 1) * sizeof(PendingWord));
    if (!temp) {
        fprintf(stderr, "Failed to allocate memory for pending words\n");
        free_memory();
        exit(EXIT_FAILURE);
    }

    pending_words = temp;
    strncpy(pending_words[pending_count].label, label, MAX_LABEL_LENGTH - 1);
    pending_words[pending_count].label[MAX_LABEL_LENGTH - 1] = '\0';
    pending_words[pending_count].address = address;
    pending_words[pending_count].mode = mode;
    pending_count++;
}

PendingWord* get_pending_words(void) {
    return pending_words;
}

int get_pending_count(void) {
    return pending_count;
}
