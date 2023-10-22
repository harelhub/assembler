#include "general.h"

FILE* pre_processor(FILE* source_file, char source_file_name[], char *full_file_name);
void assembler_both_passes(FILE *spanned_file, char *source_file_name, char *full_file_name);

/* Main function get in command line names of assembly files as parameters without ".as" postfix. It scans each
   file, let's say its name is "ababa.as", and prints a list of errors if it found an error in the file. If no
   error was found in the file, 4 (maximum) files are created as a result:
   1). ababa.am, which is ababa.as after macros were spanned.
   2). ababa.ob, which is the memory image of ababa.as program code in special binary base
   3). ababa.ent, which is a list of all labels in ababa.as which were declared with ".entry" (file is not created if there is no such label)
   4). ababa.ext, which is a list of all labels in ababa.as which were declared with ".extern" (file is not created if there is no such label)
   Assembler assumes:
   1- maximum length of assembly line is 80 chars (excluding '\n')
   2- maximum memory capacity is 256 words.
 */
int main(int argc, char* argv[]){
    FILE *file_p; /* will point on source file */
    FILE *file_p2; /* will point on spanned source file */
    int i;
    char *full_file_name;

    for(i=1; i<= (argc-1); i++) {
        full_file_name = (char *) malloc(
                strlen(argv[i]) + 4 + 1); /* 4 bytes for longest postfix like ".ent" and 1 byte for '\0' */
        if (!full_file_name) {
            printf("Failing in memory allocation while running on file %s.as\n", argv[i]);
            continue;
        }
        strcpy(full_file_name, argv[i]);
        strcpy((full_file_name + strlen(argv[i])), ".as"); /* remember strcpy also puts '\0' after ".as" */
        file_p = fopen(full_file_name, "r");
        if (!file_p) {
            printf("Failing in opening %s.as\n", argv[i]);
            free(full_file_name);
            continue;
        }
        file_p2 = pre_processor(file_p, argv[i], full_file_name);
        fclose(file_p); /* after pre_processor, we don't need source file (the not spanned one) any more */
        if(file_p2 != NULL) {
            assembler_both_passes(file_p2, argv[i], full_file_name);
            fclose(file_p2);
        }
        /* if file_p2 is NULL, then pre_processor didn't open a file (or opened but removed it), so no need to close */
        free(full_file_name);
    }
    return 0;
}
