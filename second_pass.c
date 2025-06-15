#include "pre_prossecor.h"
#include "table.h"
#include "util.h"
#include "second_pass.h"

/* Helper function to encode a data word into 24-bit binary */
unsigned int encode_data_word(DataWord dw);

/* Second pass:
   - Updates entry and data addresses
   - Finalizes object image
   - Writes .ob, .ent, .ext files */
void second_pass(const char *filename, int IC, int DC) {
    char base_name[MAX_NAME_FILE];
    char *dot = strstr(filename, ".am");

    if (dot) {
        *dot = '\0';
    }

    /* Update addresses in the entry table based on the symbol table */
    update_entry_addresses();

    /* Resolve all pending operand words (with labels) */
    update_data_words();

    /* Write final object file */
    strcpy(base_name, filename);
    strcat(base_name, ".ob");
    write_object_file(base_name, IC - MEMORY_START, DC);

    /* Write .ent file (entry symbols) */
    strcpy(base_name, filename);
    strcat(base_name, ".ent");
    write_entries_file(base_name);

    /* Write .ext file (external symbols used) */
    strcpy(base_name, filename);
    strcat(base_name, ".ext");
    write_externals_file(base_name);
}

/* Updates entry table with actual addresses from the symbol table */
void update_entry_addresses(void) {
    int i, j;
    Symbol *symbols = get_symbol_table();
    int scount = get_symbol_count();
    Entry *entries = get_entry_table();
    int ecount = get_entry_count();

    for (i = 0; i < ecount; i++) {
        for (j = 0; j < scount; j++) {
            if (!strcmp(entries[i].label, symbols[j].label)) {
                entries[i].address = symbols[j].address;
                break;
            }
        }
    }
}

/* Fills in pending operand words (that depended on labels) */
void update_data_words(void) {
    int i;
    PendingWord *pw = get_pending_words();
    int pw_count = get_pending_count();
    Object *objects = get_object_table();
    int obj_count = get_object_count();

    for (i = 0; i < pw_count; i++) {
        const char *label = pw[i].label;
        int usage_ic = pw[i].address;
        AddressingMode mode = pw[i].mode;
        int label_addr = resolve_direct_address(label);
        DataWord dw = {0};

        if (mode == RELATIVE) {
            /* Calculate relative distance from instruction */
            int distance = label_addr - (usage_ic + 1);
            if (distance < 0) {
                distance = (1 << 21) + distance;
            }
            dw.value = distance;
            dw.A = 1;
        } else if (is_external_label(label)) {
            /* External labels get 0 value and E=1 */
            dw.value = 0;
            dw.E = 1;
            add_extern(label, usage_ic);
        } else {
            /* Regular label reference (R=1) */
            dw.value = label_addr & 0x1FFFFF;
            dw.R = 1;
        }

        /* Check bounds and write encoded value to object table */
        if (usage_ic - MEMORY_START >= 0 && usage_ic - MEMORY_START < obj_count) {
            unsigned int packed = 0;
            packed |= (dw.value & 0x1FFFFF) << 3;
            packed |= (dw.A & 1) << 2;
            packed |= (dw.R & 1) << 1;
            packed |= (dw.E & 1);
            objects[usage_ic - MEMORY_START].value = packed;
        } else {
            fprintf(stderr, "Error: usage_ic %d out of range\n", usage_ic);
        }
    }
}

/* Encodes a DataWord struct into a 24-bit integer (used in update_data_words) */
unsigned int encode_data_word(DataWord dw) {
    unsigned int encoded = 0;
    encoded |= (dw.value & 0x1FFFFF) << 3;
    encoded |= (dw.A & 0x1) << 2;
    encoded |= (dw.R & 0x1) << 1;
    encoded |= (dw.E & 0x1);
    return encoded;
}

/* Given a memory address, returns the label that matches it (if any) */
const char* resolve_label_by_address(int address) {
    int i;
    Symbol *symbols = get_symbol_table();
    int count = get_symbol_count();

    for (i = 0; i < count; i++) {
        if (symbols[i].address == address) {
            return symbols[i].label;
        }
    }
    return NULL;
}

/* Returns 1 if the operand is a relative label (starts with &) */
int is_relative_label(const char *operand) {
    if (operand && operand[0] == '&') {
        return 1;
    }
    return 0;
}

/* Returns 1 if the label is defined as external */
int is_external_label(const char *label) {
    int i;
    Extern *externs = get_extern_table();
    int count = get_extern_count();

    for (i = 0; i < count; i++) {
        if (strcmp(externs[i].symbol, label) == 0) {
            return 1;
        }
    }
    return 0;
}
