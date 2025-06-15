#include "pre_prossecor.h"
#include "second_pass.h"

/* Global macro table to store defined macros */
Macro macroTable[MAX_MACROS];
int macroCount = 0;

/* List of valid instructions — macro names cannot be one of these */
const char *validInstructions[] = {
    "mov", "cmp", "add", "sub", "lea",
    "clr", "not", "inc", "dec", "jmp", "bne", "jsr", "red", "prn",
    "rts", "stop"
};

#define NUM_INSTRUCTIONS (sizeof(validInstructions) / sizeof(validInstructions[0]))

/* Check if a given macro name is valid (not an instruction) */
int isValidMacroName(char *name) {
    int i;
    for (i = 0; i < NUM_INSTRUCTIONS; i++) {
        if (strcmp(name, validInstructions[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

/* Handle macro expansion and run first and second pass if no macro errors are found */
void macro_handle(FILE *fp, char *filename) {
    int IC = MEMORY_START;                  /* Instruction counter */
    int DC = 0;                             /* Data counter */
    char line[MAX_LINE_LEN];               /* Buffer to read lines */
    int insideMacro = 0;                   /* Flag for being inside a macro */
    char macroName[MAX_MACRO_NAME];        /* Name of the current macro */
    int lineCount = 0;                     /* Number of lines inside a macro */
    int i, j;
    FILE *fp_am;                           /* Output file for macro-expanded code */
    char new_filename[MAX_NAME_FILE];      /* Name for output .am file */
    char *dot;
    int errors = 0;                        /* Counter for macro-related errors */

    /* Create new file name with .am extension */
    strcpy(new_filename, filename);
    dot = strstr(new_filename, ".as");
    if (dot) {
        *dot = '\0';
    }
    strcat(new_filename, ".am");

    /* Open .am file for writing the output after macro expansion */
    fp_am = fopen(new_filename, "w");
    if (!fp_am) {
        fprintf(stderr, "Error: Cannot create %s\n", new_filename);
        return;
    }

    /* Read input file line by line */
    while (fgets(line, MAX_LINE_LEN, fp)) {
        char firstWord[MAX_MACRO_NAME], secondWord[MAX_MACRO_NAME];
        int numWords;

        /* Check for line too long */
        if (strlen(line) >= MAX_LINE_LEN - 1) {
            fprintf(stderr, "Error: Line exceeds %d characters in file %s\n", MAX_LINE_LEN, filename);
            errors++;
            continue;
        }

        /* Try to read the first two words from the line */
        numWords = sscanf(line, "%s %s", firstWord, secondWord);

        /* Check if the line matches a macro name — if so, expand it */
        for (i = 0; i < macroCount; i++) {
            if (strcmp(firstWord, macroTable[i].name) == 0) {
                for (j = 0; j < macroTable[i].lineCount; j++) {
                    fprintf(fp_am, "%s", macroTable[i].lines[j]);
                }
                goto next_line;
            }
        }

        /* Check if line starts a new macro */
        if (strcmp(firstWord, "mcro") == 0) {
            if (numWords != 2 || !isValidMacroName(secondWord)) {
                fprintf(stderr, "Error: Invalid macro name '%s' in file %s\n", secondWord, filename);
                errors++;
                continue;
            }
            insideMacro = 1;
            strcpy(macroName, secondWord);
            lineCount = 0;
            continue;
        }

        /* Check if line ends a macro definition */
        if (strcmp(firstWord, "mcroend") == 0) {
            if (numWords != 1) {
                fprintf(stderr, "Error: 'mcroend' must be the only word in the line (file %s)\n", filename);
                errors++;
                continue;
            }
            insideMacro = 0;
            macroTable[macroCount].lineCount = lineCount;
            strcpy(macroTable[macroCount].name, macroName);
            macroCount++;
            continue;
        }

        /* If inside macro definition, save the line */
        if (insideMacro) {
            if (lineCount >= MAX_MACRO_LINES) {
                fprintf(stderr, "Error: Too many lines in macro %s\n", macroName);
                errors++;
                continue;
            }
            strcpy(macroTable[macroCount].lines[lineCount], line);
            lineCount++;
            continue;
        }

        /* Regular line — write as-is to the output .am file */
        fprintf(fp_am, "%s", line);

    next_line:;
    }

    /* Close the .am file */
    fclose(fp_am);

    /* Check if macro was opened but not closed */
    if (insideMacro) {
        fprintf(stderr, "Error: macro '%s' was not closed with 'mcroend' in file %s\n", macroName, filename);
        errors++;
    }

    /* If there were errors during macro processing, stop and delete the output file */
    if (errors > 0) {
        fprintf(stderr, "Total %d errors found. Aborting assembly for file %s\n", errors, filename);
        remove(new_filename);
        return;
    }

    /* If no macro errors, continue to first and second pass */
    first_pass(new_filename, &IC, &DC);
    second_pass(new_filename, IC, DC);
}
