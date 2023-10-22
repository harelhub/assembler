#include "general.h"
#include "tables.h"
#include "assembler.h"

void handling_command_operands_phase1(char storage[], opcode command_opcode, char program_line_copy[], flag *err_p, int instructions[], int *ic_p, int dc, char *source_file_name, command commands_table[], used_labels_table_item* *head_used_labels_table_p, flag *allocate_err_p, int num_line);

void validate_and_handling_labels_of_extern_or_entry(char storage[], class classification, labels_table_item* *head_labels_table_p, entry_labels_table_item* *head_entry_labels_table_p, flag *err_p, char *source_file_name, char program_line_copy[], flag *allocate_err_p, command commands_table[], int num_line);

void validate_and_encode_operands_of_data_or_string(char storage[], class classification, int data[], int *dc_p, flag *err_p, char program_line_copy[], char *source_file_name, int num_line);

void validate_and_analyze_line_without_its_operands(char program_line[], flag *has_label_p,command commands_table[], flag *err_p, class *classification_p, char label_str[], char storage[], opcode *command_opcode_p, char *source_file_name, char program_line_copy[], int num_line);

/**
 * converts a given decimal number to special binary base and store it in a given array as a string
 * @param num a given decimal number to be converted to special binary base
 * @param special_binary_str an array to store within the representation in special binary base of the given decimal number
 */
void convert_decimal_num_to_special_binary_str(int num, char special_binary_str[]){
    int i;
    int remainder;
    int num_copy = num; /* a copy of num because we will need to check later if num is negative and we may change num till this check happen */

    special_binary_str[14] = '\0';
    if(num<0)
        num = -num; /* if num is negative, turn it to positive and later turn its binary representation according to 2's complement */
    for(i=0; i<=13; i++)
        special_binary_str[i] = '.';
    i = 13;
    while(i >= 0) { /* we copy num bits to special_binary_str[13] to special_binary_str[0] so printing it later will be in correct order */
        remainder = num % 2; /* remainder now contain most right bit of num */
        special_binary_str[i] = ( (remainder==0)? '.':'/' );
        if ((num = num / 2) == 0) /* if num/2 equals 0 we can break from while loop since we have already initialized special_binary_str with '.' chars */
            break;
        i--;
    }
    if (num_copy < 0){
        /* turn its positive binary representation according to 2's complement by inverting all bits from the most right '/' bit till the msb (excluding this '/') */
        i = 13;
        while(special_binary_str[i--] != '/'){
        }
        while(i>=0){
            special_binary_str[i] = ( (special_binary_str[i]=='.')? '/':'.' );
            i--;
        }
    }
}

/**
 * check if a given label name is a reserved word in Assembly or not.
 * @param label_name the label name to be checked whether it's a reserved word. Ends with '\0'. Without white chars before.
 * @param commands_table a table containing details about the various assembly commands
 * @return ON if the given label_name is a reserved word, OFF if it's not
 */
flag check_label_if_reserved(char *label_name, command commands_table[]){
    int j;

    if( label_name[0]=='r' && (label_name[1])>='0' && (label_name[1])<='7' ) /* label name is a name of a register */
        return ON;
    for(j=0; j<COMMANDS_NUMBER; j++){
        if( !strcmp(label_name, commands_table[j].command_name) ) /* label name is a name of command */
            return ON;
    }
    return OFF;
}

/**
 * checks if a given label should be added to labels_table/entry_labels_table or that it has been already
   added. Prints error if necessary.
 * @param head_labels_table a pointer to the head of labels_table
 * @param label_str the label name to be checked if exist already
 * @param label_status LABEL_DEFINITION if checked label appears as definition of label in the current line.
                       LABEL_EXTERN/LABEL_ENTRY if checked label appears as an operand of extern/entry line in the current line (respectively).
 * @param source_file_name the name of source file (without ".as")
 * @param program_line_copy a copy of current program line, ends with '\0' instead of '\n', for error prints
 * @param num_line the number of current program line
 * @return ON in 3 cases: 1). given label is defined in the current program line but has been already defined
           earlier or declared as external 2). given label is declared as external in current program line but
           it has been already defined in program or declared as external or declared as entry 3). given label
           is declared as entry in current program line but it has been already declared as extern or as entry.
           Returns OFF if there is no such conflict.
 */
flag check_label_existing_before_adding(labels_table_item *head_labels_table, entry_labels_table_item *head_entry_labels_table, char *label_str, add_label_status label_status, char *source_file_name, char program_line_copy[], int num_line){
    switch (label_status){
        case LABEL_DEFINITION: /* current line tries to define this label */
            if (search_labels_table(head_labels_table, label_str)) { /*i.e. label exists already in labels table */
                if (external_field_labels_table(head_labels_table, label_str) == ON)
                    printf("File: %s.am, line %d: \"%s\", error: can't define label, because this label has been already declared as external\n",
                           source_file_name, num_line, program_line_copy);
                else /* external field is OFF */
                    printf("File: %s.am, line %d: \"%s\", error: can't define label, because this label has been already defined in program\n",
                           source_file_name, num_line, program_line_copy);
                return ON;
            }
            else /* label doesn't exist already in labels table */
                return OFF;
        case LABEL_EXTERN:  /* current line tries to declare this label as external */
            if (search_labels_table(head_labels_table, label_str)) { /*i.e. label exists already in labels table */
                if (external_field_labels_table(head_labels_table, label_str) == ON)
                    printf("File: %s.am, line %d: \"%s\", error: can't declare label as external, because this label has been already declared as external\n",
                           source_file_name, num_line, program_line_copy);
                else /* external field is OFF */
                    printf("File: %s.am, line %d: \"%s\", error: can't declare label as external, because this label has been already defined in program\n",
                           source_file_name, num_line, program_line_copy);
                return ON;
            }
            else { /* label doesn't exist already in labels table */
                if (search_entry_labels_table(head_entry_labels_table,
                                              label_str)) { /* i.e. label exists in entry_labels_table */
                    printf("File: %s.am, line %d: \"%s\", error: can't declare label as external, because this label has been already declared as entry\n",
                           source_file_name, num_line, program_line_copy);
                    return ON;
                }
                else /* label doesn't exist neither in labels_table nor in entry_labels_table */
                    return OFF;
            }
        case LABEL_ENTRY:  /* current line tries to declare this label as entry */
            if (search_labels_table(head_labels_table, label_str)) { /*i.e. label exists already in labels table */
                if (external_field_labels_table(head_labels_table, label_str) == ON) {
                    printf("File: %s.am, line %d: \"%s\", error: can't declare label as entry, because this label has been already declared as external\n",
                           source_file_name, num_line, program_line_copy);
                    return ON;
                }
            }
            /* if we got here, label doesn't exist in labels_table or that it does exist but label's "external"
               field is OFF (so it is not a problem to have now a line in program which declares this label as
               entry). We should just check that this label has not been already declared as entry. */
            if (search_entry_labels_table(head_entry_labels_table,
                                              label_str)) { /* i.e. label exists in entry_labels_table */
                printf("File: %s.am, line %d: \"%s\", error: can't declare label as entry, because this label has been already declared as entry\n",
                           source_file_name, num_line, program_line_copy);
                return ON;
                }
            return OFF;
            }
    return OFF; /* we will never reach here. This is just due to compiler warning */
    }


/**
 * free labels_table, used_labels_table and entry_labels_table
 * @param head_labels_table_p a pointer to the pointer of the head of labels_table
 * @param head_used_labels_table_p a pointer to the pointer of the head of used_labels_table
 * @param head_entry_labels_table_p a pointer to the pointer of the head of entry_labels_table
 */
void free_tables(labels_table_item* *head_labels_table_p, used_labels_table_item* *head_used_labels_table_p, entry_labels_table_item* *head_entry_labels_table_p){
    free_labels_table(head_labels_table_p);
    free_used_labels_table(head_used_labels_table_p);
    free_entry_labels_table(head_entry_labels_table_p);
}

/**
 * Scans spanned source file, let's say its name is "ababa.am", and if no error was found, 3 (maximum) files are created as a result:
   1). ababa.ob, which is the memory image of ababa.am program code in special binary base
   2). ababa.ent, which is a list of all labels in ababa.am which were declared with ".entry" (file is not created if there is no such label)
   3). ababa.ext, which is a list of all labels in ababa.am which were declared with ".extern" (file is not created if there is no such label)
   Assembler assumes:
   1- maximum length of assembly line is 80 chars (excluding '\n')
   2- maximum memory capacity is 256 words.
 * @param spanned_file a pointer to the spanned file (which was created by pre_processor)
 * @param source_file_name the name of source file (without ".as" postfix)
 * @param full_file_name the name of source file (with ".am" postfix)
 */
void assembler_both_passes(FILE *spanned_file, char *source_file_name, char *full_file_name){
/* variables definitions */
    static command commands_table[17] = {
            {"mov", MOV_CODE, {ON, ON, OFF, ON}, {OFF, ON, OFF, ON}},
            {"cmp", CMP_CODE, {ON, ON, OFF, ON}, {ON, ON, OFF, ON}},
            {"add", ADD_CODE, {ON, ON, OFF, ON}, {OFF,ON, OFF, ON}},
            {"sub", SUB_CODE, {ON, ON, OFF, ON}, {OFF,ON, OFF, ON}},
            {"not", NOT_CODE, {OFF, OFF, OFF, OFF}, {OFF, ON, OFF, ON}},
            {"clr", CLR_CODE, {OFF, OFF, OFF, OFF}, {OFF, ON, OFF, ON}},
            {"lea", LEA_CODE, {OFF, ON, OFF, OFF}, {OFF, ON, OFF, ON}},
            {"inc", INC_CODE, {OFF, OFF, OFF, OFF}, {OFF, ON, OFF, ON}},
            {"dec", DEC_CODE, {OFF, OFF, OFF, OFF}, {OFF, ON, OFF, ON}},
            {"jmp", JMP_CODE, {OFF, OFF, OFF, OFF}, {OFF, ON, ON, ON}},
            {"bne", BNE_CODE, {OFF, OFF, OFF, OFF}, {OFF, ON, ON, ON}},
            {"red", RED_CODE, {OFF, OFF, OFF, OFF}, {OFF, ON, OFF, ON}},
            {"prn", PRN_CODE, {OFF, OFF, OFF, OFF}, {ON, ON, OFF, ON}},
            {"jsr", JSR_CODE, {OFF, OFF, OFF, OFF}, {OFF, ON, ON, ON}},
            {"rts", RTS_CODE, {OFF, OFF, OFF, OFF}, {OFF, OFF, OFF, OFF}},
            {"stop", STOP_CODE, {OFF, OFF, OFF, OFF}, {OFF, OFF, OFF, OFF}},
            {NULL, NONE, {OFF, OFF, OFF, OFF}, {OFF, OFF, OFF, OFF} }
    };

    char program_line[MAX_LINE] = {0}; /* will hold a line from spanned file. Ends with '\n'. There is no really a need to initialize with '\0', it is just due to valgrind error "conditional jump or move depends on uninitialized value" */
    char program_line_copy[MAX_LINE]; /* will be used for prints of errors. Ends with '\0'. */
    int c, m , t;
    int ic = 0, dc = 0 ;
    int num_line = 0; /* number of current program line  */
    flag err = OFF;  /* holds ON if we found an error in spanned file so far, OFF otherwise. ON and OFF are defined in assembler_both_passes.h) */
    flag has_label; /* ON if the current line start with a label, OFF otherwise. ON and OFF are defined in assembler_both_passes.h) */
    char label_str[MAX_LABEL+1];  /* will hold the label string (if the line starts with a label) without ':' char. Ends with '\0' */
    class classification; /* classification of current line (class is defined in assembler_both_passes.h) */
    int instructions[256]; /* will contain code image */
    int data[256];  /* will contain data image */
    char storage[MAX_LINE]; /* an array to store within the part of line which comes after instruction word. Ends with '\n'. Array can start with space/tab chars. */
    char special_binary_str[15]; /* will hold the translation of decimal number to special 2-base of '.' and '/' */
    labels_table_item *head_labels_table = NULL; /*this table will contain all labels which are defined in program or
                                                   declared as extern (without labels declared as entry which are in entry_labels_table)*/
    used_labels_table_item *head_used_labels_table = NULL; /* an auxiliary array for second pass of assembler. Keep information about operands in program which represent labels */
    entry_labels_table_item *head_entry_labels_table = NULL; /* an auxiliary array for second pass of assembler. Keep information about labels which declared as entry in program */
    flag allocate_err = OFF; /* ON if we encountered a memory allocation error, OFF otherwise */
    opcode command_opcode; /* will hold opcode value of the command in case the current line is a command line
                              (see opcode values in assembler_both_passes.h) */
    FILE *file_p;

    /* end of variables definitions */

    rewind(spanned_file);
    while( (c=fgetc(spanned_file))!= EOF ){
    	num_line++;
        if(c=='\n') /* line with only '\n'. We can skip it. */
            continue;
        t = 0;
        program_line[t++] = c;
        while( ((c= fgetc(spanned_file))!='\n') && c!=EOF ){
            program_line[t] = c;
            t++;
        }
        if(c=='\n')
            program_line[t] = c;
        /* in case EOF comes in the end of a line (and not after a '\n') we read a '\n' instead, because our
           algorithm is based on '\n' in end of every line, and then make sure the next char we will read in the
           next while iteration will be the EOF */
        else{ /* c is EOF */
            program_line[t] = '\n';
            fseek(spanned_file, -1, SEEK_CUR);
        }
        /* in this phase, the current line is in program_line array and it ends with '\n' */
        strcpy(program_line_copy, program_line);
        (*strchr(program_line_copy, '\n')) = '\0';  /* replace '\n' with '\0' for future error prints */
        has_label = OFF; /* initialize has_label to "there is no definition of a label in current program line" */
        validate_and_analyze_line_without_its_operands(program_line, &has_label, commands_table, &err, &classification, label_str, storage, &command_opcode, source_file_name, program_line_copy, num_line);
        if(classification == EMPTY_LINE_OR_COMMENT)
            continue;  /* skip to next iteration of while loop, to get next line */
        if(classification == UNDEFINED){
            /*this if below is in order to add label to labels table despite the instruction word in the line is
              undefined, so we'll be able to print error in encounter with this label name again in program */
            if(has_label == ON){
                if(check_label_existing_before_adding(head_labels_table, head_entry_labels_table, label_str, LABEL_DEFINITION, source_file_name, program_line_copy, num_line) == ON ) {
                    /* print of reason to error is already done in check_label_existing_before_adding calling */
                    err = ON;
                }
                else { /* label doesn't exist already in labels table, so we should add it */
                    if (!add_item_labels_table(&head_labels_table, label_str, GARBAGE_VALUE, OFF, OFF, OFF, OFF)) {
                        printf("Failing in memory allocation while running on file %s.as\n", source_file_name);
                        free_tables(&head_labels_table, &head_used_labels_table, &head_entry_labels_table);
                        return;
                    }
                }
            }
            continue;  /* skip to next iteration of while loop, to get next line */
        }
        /* if we got here, classification is DATA/STRING/EXTERN/ENTRY/COMMAND */
        if( (classification == DATA) || (classification == STRING) ){
            if(has_label == ON){
                if(check_label_existing_before_adding(head_labels_table, head_entry_labels_table, label_str, LABEL_DEFINITION, source_file_name, program_line_copy, num_line) == ON ) {
                    /* print of reason to error is already done in check_label_existing_before_adding calling */
                    err = ON;
                }
                else { /* label doesn't exist already in labels table, so we should add it */
                    if (!add_item_labels_table(&head_labels_table, label_str, dc + INITIAL_ADDRESS, OFF, ON, OFF, OFF)) {
                        printf("Failing in memory allocation while running on file %s.as\n", source_file_name);
                        free_tables(&head_labels_table, &head_used_labels_table, &head_entry_labels_table);
                        return;
                    }
                }
            }
            validate_and_encode_operands_of_data_or_string(storage, classification, data, &dc, &err, program_line_copy, source_file_name, num_line);
            continue;  /* skip to next iteration of while loop, to get next line */
        }
        /* if we got here, classification is EXTERN/ENTRY/COMMAND */
        if( (classification == EXTERN) || (classification == ENTRY) ){
            /* we don't check if there is a label in the beginning of an extern/entry line, because the
               policy of this assembler is to ignore them as explained in project's instructions */
            validate_and_handling_labels_of_extern_or_entry(storage, classification, &head_labels_table, &head_entry_labels_table, &err, source_file_name, program_line_copy, &allocate_err, commands_table, num_line);
            if(allocate_err == ON) {
                printf("Failing in memory allocation while running on file %s.as\n", source_file_name);
                free_tables(&head_labels_table, &head_used_labels_table, &head_entry_labels_table);
                return;
            }
            continue;  /* skip to next iteration of while loop, to get next line */
        }
        /* if we got here, classification is COMMAND for sure */
        if(has_label == ON){
            if(check_label_existing_before_adding(head_labels_table, head_entry_labels_table, label_str, LABEL_DEFINITION, source_file_name, program_line_copy, num_line) == ON ) {
                /* print of reason to error is already done in check_label_existing_before_adding calling */
                err = ON;
            }
            else { /* label doesn't exist already in labels table, so we should add it */
                if (!add_item_labels_table(&head_labels_table, label_str, ic + INITIAL_ADDRESS, ON, OFF, OFF, OFF)) {
                    printf("Failing in memory allocation while running on file %s.as\n", source_file_name);
                    free_tables(&head_labels_table, &head_used_labels_table, &head_entry_labels_table);
                    return;
                }
            }
        }
        handling_command_operands_phase1(storage, command_opcode, program_line_copy, &err, instructions, &ic, dc, source_file_name, commands_table, &head_used_labels_table, &allocate_err, num_line);
        if(allocate_err == ON) {
            printf("Failing in memory allocation while running on file %s.as\n", source_file_name);
            free_tables(&head_labels_table, &head_used_labels_table, &head_entry_labels_table);
            return;
        }
    }

    /* let's add final ic value to addresses of data labels before we go to second pass of assembler */
    add_to_address_labels_table(head_labels_table, ic);

    /* SECOND PASS OF ASSEMBLER - it is not going really to be a whole new pass on program since we deliberately
       built in the first pass the tables: used_labels_tables and entry_labels_table. These tables keep the
       specific changes/updates we have to do in second pass and avoid us from going through the program again
       and find it ourselves */

    handling_used_labels_table(head_used_labels_table, head_labels_table, instructions, &err, source_file_name);
    handling_entry_labels_table(head_entry_labels_table, head_labels_table, &err, source_file_name);

    if(err == ON) {
        free_tables(&head_labels_table, &head_used_labels_table, &head_entry_labels_table);
        return;
    }
    /* if we got here, there was no error in program. Now let's create output files */

    strcpy(strrchr(full_file_name, '.'), ".ob"); /* replacing ".am" postfix with ".ob" postfix */
    file_p = fopen(full_file_name, "w");
    if(!file_p){
        printf("Failing in creating %s\n", full_file_name);
        free_tables(&head_labels_table, &head_used_labels_table, &head_entry_labels_table);
        return;
    }
    fprintf(file_p, "%d %d\n", ic, dc); /* headline (i.e. first line) of .ob file */
    /* printing code segment */
    m = 0;
    while(m < ic){
        fprintf(file_p, "%04d\t\t", (INITIAL_ADDRESS + m));
        convert_decimal_num_to_special_binary_str(instructions[m], special_binary_str);
        fputs(special_binary_str, file_p);
        fputc('\n', file_p);
        m++;
    }
    /* printing data segment */
    m = 0;
    while(m < dc){
        fprintf(file_p, "%04d\t\t", (INITIAL_ADDRESS + ic + m));
        convert_decimal_num_to_special_binary_str(data[m], special_binary_str);
        fputs(special_binary_str, file_p);
        fputc('\n', file_p);
        m++;
    }
    fclose(file_p);

    /* creating ".ent" file */
    strcpy(strrchr(full_file_name, '.'), ".ent"); /* replacing ".ob" postfix with ".ent" postfix */
    if( create_entry_file_from_entry_labels_table(head_entry_labels_table, full_file_name) == OFF ){ /* process failed */
        /* error print was already done in called function */
        strcpy(strrchr(full_file_name, '.'), ".ob"); /* replacing ".ent" postfix with ".ob" postfix */
        remove(full_file_name);
        free_tables(&head_labels_table, &head_used_labels_table, &head_entry_labels_table);
        return;
    }
    /* creating ".ext" file */
    strcpy(strrchr(full_file_name, '.'), ".ext"); /* replacing ".ent" postfix with ".ext" postfix */
    if( create_extern_file_from_used_labels_table(head_used_labels_table, full_file_name) == OFF ){ /* process failed */
        /* error print was already done in called function */
        strcpy(strrchr(full_file_name, '.'), ".ob"); /* replacing ".ext" postfix with ".ob" postfix */
        remove(full_file_name);
        if(head_entry_labels_table){
        	strcpy(strrchr(full_file_name, '.'), ".ent"); /*if entry_labels_table is not empty, replace ".ob" postfix with ".ent"*/
        	remove(full_file_name);
        }
        free_tables(&head_labels_table, &head_used_labels_table, &head_entry_labels_table);
        return;
    }

    free_tables(&head_labels_table, &head_used_labels_table, &head_entry_labels_table);
    return;
}
