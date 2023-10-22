#include "general.h"
#include "tables.h"
#include "assembler.h"

/**
 * Extracts the first word of the line into first_word_in_line parameter.
 * @param program_line array which contains a program line (ends with '\n'. May has other chars after '\n' like '\0' but they aren't considered).
 * @param first_word_in_line array to where function stores the first word of line that is extracted. If
                             line is only white chars, first_word_in_line[0] will hold '\0'.
 */
void extract_first_word_of_line(char program_line[], char first_word_in_line[]){
    int c;
    int k=0, m=0;

    while( ((c=program_line[k++]) == ' ') || (c == '\t') )
    {}
    k--;  /* k was incremented by 1, so program_line[k] pointed to a char that comes after(!) the char in
             c. Now it is pointing to the char in c. */
    while( ((c=program_line[k++])!=' ') && c!= '\t' && c!= '\n' ){
        first_word_in_line[m] = c;
        m++;
    }
    first_word_in_line[m] = '\0';
}

/**
 * span all macros and create a spanned source program, provided there is no error in macro opening/ending lines
 * @param source_file a pointer to the source file
 * @param source_file_name name of the source file (without .as postfix)
 * @param full_file_name name of the source file (with .as postfix)
 * @return a pointer to the spanned program file that is created (if created)
 */
FILE* pre_processor(FILE* source_file, char source_file_name[], char *full_file_name){
    char program_line[MAX_LINE + 1]; /* will hold a line we read from source program. +1 is for '\0'. */
    char first_word_in_line[MAX_LINE]; /*will hold the first word of the line we read from source program. Ends
                                         with '\0'. Size of macro name is not limited, hence took array of MAX_LINE bytes */
    char second_word[MAX_LINE]; /* will hold macro name word after mcr word in macro opening line in program */
    char spare[MAX_LINE];
    int c; /* for holding a char */
    int t, r; /* just simple indexes */
    int num, macro_size; /* for number of program lines inside macro body */
    int counter = 0; /* counts source program lines */
    flag mcr = OFF; /* ON if we are in the middle of macro definition and OFF otherwise. ON and OFF constants
                         are defined in assembler_both_passes.h */
    char *macro_content;
    macros_table_item *macro_item_p;
    macros_table_item *head_of_macros_table = NULL;
    FILE *spanned_program;
    flag err = OFF; /* ON if we encounter a char after name of macro in mcr line or after endmcr in endmcr line */
    
    /* end of variables definitions */

    strcpy(strrchr(full_file_name, '.'), ".am"); /* replacing ".as" postfix with ".am" postfix */
    spanned_program = fopen(full_file_name, "w+");
    if(!spanned_program){
        printf("Failing in opening %s.am\n", source_file_name);
        return NULL;
    }

    while( (c=fgetc(source_file)) != EOF ) {
        counter++;
        if (c == '\n'){  /* means empty line */
            if(mcr == ON) {  /* (besides of empty line) it means we are in the middle of macro definition */
                strcpy(&macro_content[macro_size*(MAX_LINE+1)], "\n\0");
                macro_size++;
                macro_content = (char*)realloc(macro_content, (macro_size+1)*(MAX_LINE+1));
                if(macro_content == NULL) {
                    printf("Error in memory allocation while running on file %s.as\n", source_file_name);
                    free_macros_table(&head_of_macros_table);
                    remove(full_file_name);
                    return NULL;
                }
            }
            else /* (besides of empty line) it means we are NOT in the middle of macro definition */
                fputc('\n', spanned_program);
            continue;
        }
        /* if we got here, it's not an empty line (here we refer empty line as line with only '\n'. We can still encounter a line of only space/tab chars.)*/
        t=0;
        program_line[t++] = c;
        while( ((c=fgetc(source_file))!='\n') && (c!=EOF) ){
            program_line[t] = c;
            t++;
        }
        if(c == '\n')
            program_line[t] = c;
        /* in case EOF comes in the end of a line (and not after a '\n') we read a '\n' instead, because our
           algorithm is based on '\n' in end of every line, and then make sure the next char we will read in
           the next while iteration will be the EOF */
        else{ /* c is EOF */
            program_line[t] = '\n';
            fseek(source_file, -1, SEEK_CUR);
        }
        t++;
        program_line[t] = '\0';

        if(program_line[0] == ';'){  /* comment line */
            if(mcr == ON) {  /* (besides comment line) it means we are in the middle of macro definition */
                strcpy(&macro_content[macro_size*(MAX_LINE+1)], program_line);
                macro_size++;
                macro_content = (char*)realloc(macro_content, (macro_size+1)*(MAX_LINE+1));
                if(macro_content == NULL) {
                    printf("Error in memory allocation while running on file %s.as\n", source_file_name);
                    free_macros_table(&head_of_macros_table);
                    remove(full_file_name);
                    return NULL;
                }
            }
            else  /* (besides comment line) it means we are NOT in the middle of macro definition */
                fputs(program_line, spanned_program);
            continue;
        }
        /* if we got here, it is not an empty/comment line */
        extract_first_word_of_line(program_line, first_word_in_line);
        /* now first word of line is inside first_word_in_line array */

        if(!strcmp(first_word_in_line, "endmcr")) {  /*line is closing of macro definition (i.e. endmcr line)*/
            extract_first_word_of_line( (strchr(program_line,'r') + 1) , spare);
            if(spare[0] != '\0') {
                printf("File: %s.as, line: %d, error: invalid character after endmcr\n", source_file_name, counter);
                err = ON;
            }
            mcr = OFF;
            if(!add_item_macros_table(&head_of_macros_table, second_word, macro_content, macro_size) ) {
                printf("Error in memory allocation while running on file %s.as\n", source_file_name);
                free_macros_table(&head_of_macros_table);
                remove(full_file_name);
                return NULL;
            }
            continue; /* go to head of while loop to read the next lines in program */
        }

        /*if we got here, it is not empty/comment/endmcr line */
        /*true in if below means that first word in line is a macro name from macros table. */
        if( (macro_item_p = search_macros_table(head_of_macros_table, first_word_in_line) ) ){
            num = size_macros_table(macro_item_p);
            for(r=0; r<num; r++){
                fputs( (content_macros_table(macro_item_p) + r*(MAX_LINE+1) ), spanned_program);
            }
            continue; /* we copied macro's content and ready to skip to read the next line in source program */
        }

        /* if we got here, this is not an empty/comment/endmcr line and not macro usage line */
        /*true in if below, means it is a macro definition opening line (i.e. mcr line) */
        if(!strcmp(first_word_in_line, "mcr")) {
            mcr = ON;
            /* macro name is the first word in line if line begins from the 4th char (right after mcr word).
               Hence, we can extract macro name by the next function call */
            extract_first_word_of_line( (strchr(program_line, 'r') + 1), second_word);

            extract_first_word_of_line( (strstr(program_line, second_word) + strlen(second_word) ), spare);
            if (spare[0] != '\0'){
                printf("File: %s.as, line: %d, error: invalid character after name of macro\n", source_file_name, counter);
                err = ON;
            }
            macro_content = (char*)malloc(MAX_LINE+1);
            macro_size = 0;
            continue;  /* go to head of while loop to start read macro content (i.e. lines inside macro) */
        }

        /* if we got here, this is not an empty/comment/mcr/endmcr line and not macro usage line, i.e. just a
           regular line without any connection to macros */
        if(mcr == ON) {  /*(besided the fact it is just a regular line) it means we are in the middle of macro definition */
            strcpy(&macro_content[macro_size*(MAX_LINE+1)], program_line);
            macro_size++;
            macro_content = (char*)realloc(macro_content, (macro_size+1)*(MAX_LINE+1));
            if(macro_content == NULL) {
                printf("Error in memory allocation while running on file %s.as\n", source_file_name);
                free_macros_table(&head_of_macros_table);
                remove(full_file_name);
                return NULL;
            }
        }
        else /*(besides the fact it is just a regular line) it means we are NOT in the middle of macro definition */
            fputs(program_line, spanned_program);
        }
    	
    if(err == ON){
    	free_macros_table(&head_of_macros_table);
        remove(full_file_name);
        return NULL;	
    }	
    	
    /*if we got here, pre_processor was completed successfully and main will be responsible to close spanned file*/
    free_macros_table(&head_of_macros_table);
    return spanned_program;
    }

