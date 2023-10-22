#include "general.h"
#include "tables.h"
#include "assembler.h"

flag check_label_if_reserved(char *label_name, command commands_table[]);
flag check_label_existing_before_adding(labels_table_item *head_labels_table, entry_labels_table_item *head_entry_labels_table, char *label_str, add_label_status label_status, char *source_file_name, char program_line_copy[], int num_line);

/**
 * checks validity of label field after ".extern"/".entry" word and add label to labels_table/entry_labels_table
 * @param storage an array containing only the label field of a .extern/.entry instruction (with maybe
 *                space/tab chars in the beginning) including a '\n' char in the end.
 * @param classification holds EXTERN constant in case of ".extern" instruction, and ENTRY constant in case of
                         ".entry" instruction (these constants are defined in assembler_both_passes.h)
 * @param head_labels_table_p a pointer to the pointer of the head of labels_table
 * @param head_entry_labels_table_p a pointer to the pointer of the head of entry_labels_table
 * @param err_p a pointer to variable which holds ON if we found an error in spanned file so far, OFF otherwise.
 * @param source_file_name name of source file (without ".as" postfix)
 * @param program_line_copy a copy of current program line
 * @param allocate_err_p a pointer to variable which holds ON if we encountered a memory allocation, OFF otherwise.
 * @param commands_table a table containing details about the various assembly commands
 * @param num_line the number of current program line
 */
void validate_and_handling_labels_of_extern_or_entry(char storage[], class classification, labels_table_item* *head_labels_table_p, entry_labels_table_item* *head_entry_labels_table_p, flag *err_p, char *source_file_name, char program_line_copy[], flag *allocate_err_p, command commands_table[], int num_line){
    int k=0, m=0;
    int c;

    while( ((c=storage[k++])==' ') || c=='\t' ){
    }
    if(c == '\n'){
        (*err_p) = ON;
        printf("File: %s.am, line %d: \"%s\", error: there is no label after instruction word\n", source_file_name, num_line, program_line_copy);
        return;
    }
    if( !(isupper(c) || islower(c)) ){
        (*err_p) = ON;
        printf("File: %s.am, line %d: \"%s\", error: label must begin with a letter\n", source_file_name, num_line, program_line_copy);
        return;
    }
    k--;  /* after this decrement, storage[k] is c (first letter char in storage array) */
    while( isupper((c=storage[k++])) || islower(c) || isdigit(c) ){
        storage[m] = c;
        m++;
    }
    storage[m] = '\0';
    k--;  /* after this decrement, storage[k] is c (first char after label chars in storage array) */
    while( ((c=storage[k++])==' ') || c=='\t' ){
    }
    if(c != '\n'){
        (*err_p) = ON;
        printf("File: %s.am, line %d: \"%s\", error: label should be a single word constructed only from letters/digits\n", source_file_name, num_line, program_line_copy);
        return;
    }
    /* if we got here, storage begins immediately with label (without white chars before) and ends with '\0' */
    if(strlen(storage) > 30){
        (*err_p) = ON;
        printf("File: %s.am, line %d: \"%s\", error: label can't be more than 30 chars\n", source_file_name, num_line, program_line_copy);
        return;
    }
    if(check_label_if_reserved(storage, commands_table)){
        (*err_p) = ON;
        printf("File: %s.am, line %d: \"%s\", error: label name can't be a reserved word\n", source_file_name, num_line, program_line_copy);
        return;
    }
    /*if we got here, the field in line which comes after .entry/.extern word is valid. Now we should handle it*/
    switch (classification) {
        case EXTERN:
            if(check_label_existing_before_adding(*head_labels_table_p, *head_entry_labels_table_p, storage, LABEL_EXTERN, source_file_name, program_line_copy, num_line)){
                /* print of reason to error is already done in check_label_existing_before_adding calling */
                (*err_p) = ON;
                return;
            }
            /* if we got here, we can safely add this label to labels_table (with external sign) */
            if( !add_item_labels_table(head_labels_table_p, storage, EXTERN_LABEL_ADDRESS, OFF, OFF, ON, OFF) )
                (*allocate_err_p) = ON;
            break;
        case ENTRY:
            if( check_label_existing_before_adding(*head_labels_table_p, *head_entry_labels_table_p, storage, LABEL_ENTRY, source_file_name, program_line_copy, num_line) ){
                (*err_p) = ON;
                /* print of reason to error is already done in check_label_existing_before_adding calling */
                return;
            }
            /* if we got here, we can safely add this label to entry_labels_table */
            /* field "address" value will be known, and therefore assigned, only in the second pass of the
               assembler. It is also not needed till then so meanwhile we assign it with garbage value */
            if( !add_item_entry_labels_table(head_entry_labels_table_p, storage, program_line_copy, num_line, GARBAGE_VALUE) ){
                (*allocate_err_p) = ON;
            }
            break;
        /* the cases below are never reached. I wrote it just because a compiler warning */    
    	case EMPTY_LINE_OR_COMMENT:
    	case DATA:
    	case STRING:
    	case COMMAND:
    	case UNDEFINED:
            break;
    }
}
