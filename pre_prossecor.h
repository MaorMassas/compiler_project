#ifndef PRE_PROSSECOR_H
#define PRE_PROSSECOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Maximum number of data items */
#define SIZE_DATA 100

/* Memory starting address for the assembler */
#define MEMORY_START 100

/* Maximum length of a file name (not including extension) */
#define MAX_NAME_FILE 20

/* Maximum allowed length for a macro name */
#define MAX_MACRO_NAME 31

/* Maximum number of lines that can be stored in a macro definition */
#define MAX_MACRO_LINES 100

/* Maximum length of a line in the input file */
#define MAX_LINE_LEN 80

/* Maximum number of macros that can be defined in one file */
#define MAX_MACROS 100

/* Maximum number of memory words (2^21 = 2097152) */
#define MAX_MEMORY 2097152

/* Maximum length for a label (not including colon) */
#define MAX_LABEL_LENGTH 31

/* Structure to hold a macro definition:
   - name: the macro name
   - lines: the lines that make up the macro body
   - lineCount: how many lines the macro contains */
typedef struct {
    char name[MAX_MACRO_NAME];
    char lines[MAX_MACRO_LINES][MAX_LINE_LEN];
    int lineCount;
} Macro;

/* Handles macro expansion in the first preprocessing step.
   - fp: pointer to the opened input file (.as)
   - filename: name of the input file (used to generate .am)
   This function extracts all macros and replaces their usage. */
void macro_handle(FILE *fp, char *filename);

/* Performs the first pass of the assembler.
   - filename: the name of the preprocessed file (.am)
   - IC: pointer to instruction counter (initially MEMORY_START)
   - DC: pointer to data counter
   This pass handles labels, directives, and validates instruction syntax. */
void first_pass(const char *filename, int *IC, int *DC);

#endif
