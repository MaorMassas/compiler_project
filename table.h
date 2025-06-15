#ifndef TABLE_H
#define TABLE_H

#include "util.h"

#define MAX_LABEL_LENGTH 31

/* Structure for storing label definitions in the symbol table */
typedef struct {
    char label[MAX_LABEL_LENGTH];     /* Label name */
    unsigned int address;             /* Address in memory */
    int data_value[SIZE_DATA];        /* Associated data (for .data/.string) */
    int data_count;                   /* Number of data values */
} Symbol;

/* Structure for storing .entry labels */
typedef struct {
    char label[MAX_LABEL_LENGTH];     /* Label name */
    int address;                      /* Address in memory */
} Entry;

/* Structure for storing .extern symbols and where they were used */
typedef struct {
    char symbol[MAX_LABEL_LENGTH];    /* External symbol name */
    int address;                      /* Address where it was used */
} Extern;

/* Structure for storing memory words of the final object image */
typedef struct {
    unsigned short address;           /* Memory address */
    unsigned int value;               /* Encoded 24-bit machine word */
} Object;

/* Structure for storing unresolved words that need to be completed in second pass */
typedef struct {
    int address;                      /* Address in memory */
    char label[MAX_LABEL_LENGTH];     /* Label name that will be resolved */
    AddressingMode mode;              /* Addressing mode (direct/relative/etc.) */
} PendingWord;

/* Adds a symbol (label) to the symbol table */
void add_symbol(const char *label, int address);

/* Adds an object word (instruction or data) to the object image */
void add_object(unsigned int address, int value);

/* Adds an external symbol reference */
void add_extern(const char *symbol, int address);

/* Adds an entry symbol (.entry directive) */
void add_entry(const char *label, int address);

/* Adds data to a symbol's data array (used in .data/.string) */
void add_data_to_symbol(int symbol_index, int value);

/* Frees all dynamically allocated memory tables */
void free_memory(void);

/* Resolves the address of a label by name from the symbol table */
int resolve_direct_address(const char *label);

/* Returns 1 if the label is external, 0 otherwise */
int is_external_label(const char *label);

/* Adds a word to be resolved later (in second pass) */
void add_pending_word(const char *label, int address, AddressingMode mode);

/* Returns the array of pending words */
PendingWord* get_pending_words(void);

/* Returns the number of pending words */
int get_pending_count(void);

/* Returns the number of defined symbols */
int get_symbol_count(void);

/* Returns the number of .entry declarations */
int get_entry_count(void);

/* Returns the number of .extern declarations */
int get_extern_count(void);

/* Returns the number of object words in memory */
int get_object_count(void);

/* Returns a pointer to the symbol table */
Symbol* get_symbol_table(void);

/* Returns a pointer to the extern table */
Extern* get_extern_table(void);

/* Returns a pointer to the entry table */
Entry* get_entry_table(void);

/* Returns a pointer to the object memory table */
Object* get_object_table(void);

/* Writes the final .ob file with object image */
void write_object_file(const char *filename, int IC, int DC);

/* Writes the .ent file with all entry symbols */
void write_entries_file(const char *filename);

/* Writes the .ext file with all external symbols used */
void write_externals_file(const char *filename);

#endif /* TABLE_H */
