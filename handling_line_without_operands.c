#include "general.h"
#include "assembler.h"

void extract_first_word_of_line(char program_line[], char first_word_in_line[]);
flag check_label_if_reserved(char *label_name, command commands_table[]);

/**
 * checks if a given first word of a line is a valid label, invalid label or not a label at all
 * @param first_word first word of a line. Ends with '\0'. Without white chars at all.
 * @param source_file_name the name of source file (without ".as" postfix)
 * @param program_line_copy a copy of current program line. Ends with '\0'.
 * @param num_line the number of current program line
 * @return NOT_LABEL first word is not a label, VALID_LABEL if first word is a valid label and INVALID_LABEL if
           first word is an invalid label (All constants are defined in assembler_both_passes.h)
 */
is_first_word_a_label checking_first_word_of_line(char *first_word, command instruction_table[], char *source_file_name, char program_line_copy[], int num_line){
   int i, j = 0;

   while(first_word[j] != ':'){
       if(first_word[j] == '\0')
           return NOT_LABEL; /* no ':' in the word so we assume this is not a definition of a label at all */
       j++;
   }
   /*if we got here, there is ':' in this word, so we assume the word is supposed to be a label, and now we check its validity*/
   if( !(isupper(first_word[0]) || islower(first_word[0])) ) {
       printf("File: %s.am, line %d: \"%s\", error: label must start with a letter\n", source_file_name, num_line, program_line_copy);
       return INVALID_LABEL;
   }
   for(i=1; i<j; i++){  /* remember that first_word[j] contains the first time of ':' in the array */
       if( !(isupper(first_word[i]) || islower(first_word[i]) || isdigit(first_word[i])) ){
           printf("File: %s.am, line %d: \"%s\", error: label must include only letters or digits\n", source_file_name, num_line, program_line_copy);
           return INVALID_LABEL;
       }
   }
   if(first_word[j+1] != '\0'){ /* remember that first_word[j] contains the first time of ':' in the array */
       printf("File: %s.am, line %d: \"%s\", error: label must have space/tab after ':'\n", source_file_name, num_line, program_line_copy);
       return INVALID_LABEL;
   }
   first_word[j] = '\0'; /* we replace ':' which comes after label with '\0' for check_if_reserved call. We retrieve ':' right after this call */
   if( check_label_if_reserved(first_word, instruction_table) == ON ){
       first_word[j] = ':';
       printf("File: %s.am, line %d: \"%s\", error: label name can't be a reserved word\n", source_file_name, num_line, program_line_copy);
       return INVALID_LABEL;
   }
   first_word[j] = ':';
   return VALID_LABEL;
}

/**
 * checks a given word if it is a known instruction word and classify it to matching instruction word type
 * @param word the word to be checked whether it is a known instruction word
 * @param classification_p a pointer to a variable holding the class of the line (info about values of class in assembler_both_passes.h)
 * @param commands_table a table with information about types of assembly commands and their valid addressing methods
 * @param command_opcode_p a pointer to opcode field of this command word (in case the given word is a command)
 */
void check_instruction_word(char *word, class *classification_p, command commands_table[], opcode *command_opcode_p){
    int j=0;

    if (!strcmp(word, DATA_INSTRUCTION)) {
        (*classification_p) = DATA;
        return;
    }
    if (!strcmp(word, STRING_INSTRUCTION)) {
        (*classification_p) = STRING;
        return;
    }
    if (!strcmp(word, ENTRY_INSTRUCTION)) {
        (*classification_p) = ENTRY;
        return;
    }
    if (!strcmp(word, EXTERN_INSTRUCTION)) {
        (*classification_p) = EXTERN;
        return;
    }
    for(j=0; j<COMMANDS_NUMBER; j++){
        if(!strcmp(word, commands_table[j].command_name)){
            (*classification_p) = COMMAND;
            (*command_opcode_p) = commands_table[j].command_code;
            return;
        }
    }
    /* if we got here, the word is not a known instruction word */
    (*classification_p) = UNDEFINED;
}


/**
 * checks validity of instruction word. check validity of label (if line starts with label). Classify line to
   empty/comment/command/data/string/extern/entry/undefined line. Extracts label (if line starts with label)
   and part of line after instruction word to given arrays.
 * @param program_line current program line (ends with '\n')
 * @param has_label_p a pointer to a flag which gets ON if line starts with label and OFF otherwise. It comes with initialization to OFF (ON and OFF are defined in assembler_both_passes.h)
 * @param commands_table a table with information about types of assembly commands and their valid addressing methods
 * @param err_p a pointer to a flag which gets ON if an error has been discovered so far in the program and OFF otherwise (ON and OFF are defined in assembler_both_passes.h)
 * @param classification_p a pointer to a variable holding the class of the line (info about values of class in assembler_both_passes.h)
 * @param label_str an array to store within the label string (if the line starts with a label at all)
 * @param storage an array to store within the part of line which comes after instruction word (i.e. operands part). Ends with '\n'. Array can start with space/tab chars.
 * @param source_file_name the name of source file (without ".as" postfix)
 * @param program_line_copy a copy of current program line for error prints
 * @param num_line the number of current program line
 */
void validate_and_analyze_line_without_its_operands(char program_line[], flag *has_label_p, command commands_table[],
                                                    flag *err_p, class *classification_p, char label_str[], char storage[], opcode *command_opcode_p, char *source_file_name, char program_line_copy[], int num_line) {
    int c, j = 0, m = 0;
    is_first_word_a_label label_status;
    char first_word[MAX_LINE]; /* will hold first word of line. MAX_LINE(81 bytes) is enough for word + '\0' */
    char second_word[MAX_LINE]; /* will hold second word of line. MAX_LINE(81 bytes) is enough for word + '\0' */

    while ((c = program_line[j]) != '\n') {
        storage[j] = c;
        j++;
    }
    storage[j] = c;
    /* now storage array contains the same as program_line array (including '\n' in the end) */
    if (storage[0] == ';') {  /* which means it is a comment line */
        (*classification_p) = EMPTY_LINE_OR_COMMENT;
        return;
    }
    j = 0;
    while (((c = storage[j]) == ' ') || c == '\t') {
        j++;
    }
    if (c == '\n') {  /* no non-white char in line, which means it is an empty line */
        (*classification_p) = EMPTY_LINE_OR_COMMENT;
        return;
    }
    /* if we got here, this is not an empty/comment line */

    extract_first_word_of_line(storage, first_word);
    /* now first_word array contains first word of line, ending with '\0' */
    /*label can not be more than 30 chars (not including ':'). Of course others words/operands (not including
      macros which we have already dealt with) too. The if condition below checks this. */
    if (strlen(first_word) > (MAX_LABEL + 1)) {
        (*classification_p) = UNDEFINED;
        (*err_p) = ON;
        printf("File: %s.am, line %d: \"%s\", error: no instruction word, including a label, can be more than 30 chars\n",
               source_file_name, num_line, program_line_copy);
        return;
    }
    /* the 2 loops below are responsible to make storage array contain only operands part of line by replacing
       chars before with ' ' chars. This is among the roles of this function as said in its description above */
    j = 0;
    while ((storage[j] == ' ') || storage[j] == '\t') {
        j++;
    }
    for (m = 0; m < strlen(first_word); m++) {
        storage[j + m] = ' ';
    }

    label_status = checking_first_word_of_line(first_word, commands_table, source_file_name, program_line_copy, num_line);
    if (label_status == INVALID_LABEL) {
        (*classification_p) = UNDEFINED;
        (*err_p) = ON;
        /* a print of the reason to error in label has been already done in the called function checking_first_word_of_line */
        return;
    }

    /* handling case in which first word is not a label */
    if (label_status == NOT_LABEL) {
        check_instruction_word(first_word, classification_p, commands_table, command_opcode_p);
        if( (*classification_p) == UNDEFINED){
            (*err_p) = ON;
            printf("File: %s.am, line %d: \"%s\", error: first field is undefined. For label, make sure there are no white chars inside label name or between label name to ':'. For instruction word, make sure to put a valid one and to put a space/tab after it.\n", source_file_name, num_line, program_line_copy);
        }
        return;
    }

    /* handling case in which first word is a label */
    if( label_status == VALID_LABEL ){
        (*has_label_p) = ON;
        strcpy(label_str, first_word);
        label_str[strlen(first_word)-1] = '\0';  /* replacing ':' with '\0'. This is a part of the function's
                                                    roles as said in its description above */
        /* now using the fact we replace first word chars in storage array with space chars, we extract the
           instruction word after label word (i.e. second word of line) and analyze it. */
        extract_first_word_of_line(storage, second_word);
        if(second_word[0] == '\0'){  /* i.e. no instruction word after label */
            (*classification_p) = UNDEFINED;
            (*err_p) = ON;
            printf("File: %s.am, line %d: \"%s\", error: no instruction word after label\n", source_file_name, num_line, program_line_copy);
            return;
        }
        check_instruction_word(second_word, classification_p, commands_table, command_opcode_p);
        if( (*classification_p) == UNDEFINED){
            (*err_p) = ON;
            printf("File: %s.am, line %d: \"%s\", error: instruction word is undefined\n", source_file_name, num_line, program_line_copy);
            return;
        }
        /* the 2 loops below are responsible to make storage array contain only operands part of line by replacing
           chars before with space chars. This is among the roles of this function as said in its description above */
        j = 0;
        while((storage[j] == ' ') || storage[j] == '\t'){
            j++;
        }
        for(m = 0; m < strlen(second_word); m++){
            storage[j + m] = ' ';
        }

        return;
    }
}
