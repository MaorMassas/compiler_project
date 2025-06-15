#ifndef SECOND_PASS_H
#define SECOND_PASS_H

/* Runs the second pass of the assembler.
 * - Resolves pending words (like labels)
 * - Updates object image
 * - Writes .ob, .ent, .ext output files
 * Parameters:
 *   output_filename_base - name of the file (without .am extension)
 *   IC - final instruction counter
 *   DC - final data counter
 */
void second_pass(const char *output_filename_base, int IC, int DC);

/* Updates addresses for all entry symbols in the entry table
 * by looking them up in the symbol table.
 */
void update_entry_addresses(void);

/* Updates the object table by filling in values for operands
 * that were delayed due to label resolution (e.g., direct, relative).
 */
void update_data_words(void);

/* Returns the label name that appears at a given address,
 * or NULL if no label has that address.
 */
const char* resolve_label_by_address(int address);

/* Checks if a given operand represents a relative label.
 * A relative label starts with '&'.
 * Returns 1 if true, 0 otherwise.
 */
int is_relative_label(const char *operand);

/* Checks if a label was declared as external using .extern.
 * Returns 1 if the label is external, 0 otherwise.
 */
int is_external_label(const char *label);

#endif /* SECOND_PASS_H */
