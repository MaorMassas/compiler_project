#include "pre_prossecor.h"

/*Maor Massas
 * 314801887*/


int main(int args, char *argv[]) {
    FILE *fp;                              /* File pointer for reading source file */
    int i;                                 /* Loop index */
    char name_of_file[MAX_NAME_FILE];     /* Buffer to store file name */

    /* Check if at least one file was provided */
    if (args < 2) {
        fprintf(stderr, "Error: No file was input\n");
        return 1;
    }

    /* Loop through all input file arguments */
    for (i = 1; i < args; i++) {

        /* If file name already ends with .as, copy it directly */
        if (strstr(argv[i], ".as") != NULL) {
            strcpy(name_of_file, argv[i]);
        } else {
            /* Otherwise, add .as extension to the file name */
            strcpy(name_of_file, argv[i]);
            strcat(name_of_file, ".as");
        }

        /* Print message for debugging */
        printf("Trying to open file: %s\n", name_of_file);

        /* Try to open the file */
        fp = fopen(name_of_file, "r");
        if (!fp) {
            fprintf(stderr, "Error: File '%s' not found\n", name_of_file);
            continue;
        }

        /* Call macro handler to preprocess macros in the file */
        macro_handle(fp, name_of_file);

        /* Close the file after processing */
        fclose(fp);

        /* Notify that processing of the file has finished */
        fprintf(stdout, "Finished processing file: %s\n", name_of_file);
    }

    return 0;
}
