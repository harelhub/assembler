#include "general.h"
#include "tables.h"
#include "assembler.h"

/* ##################### now there are functions for handling labels_table ##################### */
/**
 * checks if a given label name exist in labels_table or not
 * @param head_table a pointer to the head of labels_table
 * @param name_to_search the label name to be searched
 * @return ON if label name exists in labels_table and OFF otherwise
 */
flag search_labels_table(labels_table_item *head_table, char name_to_search[]){
    labels_table_item *temp_item = head_table;

    if(!head_table)  /* which means empty table */
        return OFF;
    while((*temp_item).next != NULL){
        if( !strcmp((*temp_item).label_name, name_to_search) ) /*true means label_name is equal to name_to_search*/
            return ON;
        temp_item = (*temp_item).next;
    }
    if( !strcmp((*temp_item).label_name, name_to_search) ) /*if we got here, only last item was not checked yet*/
        return ON;
    return OFF;
}

/**
 * Adds a label to labels_table and initialize its fields
 * @param head_table_p a pointer to pointer of the head of labels_table
 * @param label_name name of the label to be added
 * @param address address of the label to be added
 * @param command ON if the label which is added appears in a label defintion in a command line. OFF otherwise.
 * @param data ON if the label which is added appears in a label defintion in a ".data"/".string" line. OFF otherwise.
 * @param external ON if the label which is added appears in a label declaration in a ".extern" line. OFF otherwise.
 * @param entry ON if the label which is added appears in a label declaration in a ".entry" line. OFF otherwise.
 * @return ON if label was added successfully to labels_table and OFF otherwise
 */
flag add_item_labels_table(labels_table_item* *head_table_p, char label_name[], int address, flag command, flag data, flag external, flag entry){
    labels_table_item *p;
    labels_table_item *p1;

    p = (labels_table_item*)malloc(sizeof(labels_table_item));
    if(!p)
        return OFF;

    strcpy((*p).label_name, label_name);
    (*p).address = address;
    (*p).command = command;
    (*p).data = data;
    (*p).external = external;
    (*p).entry = entry;
    (*p).next = NULL;

    if((*head_table_p) == NULL) {  /* means an empty table */
        (*head_table_p) = p;
        return ON;
    }
    /* if we got here, table is not empty */
    p1 = (*head_table_p);
    while((*p1).next != NULL)
        p1 = (*p1).next;
    (*p1).next = p;
    return ON;
}

/**
 * returns address field value of a given label name (pay attention: label name must exist in labels table!)
 * @param head_table a pointer to first item in labels table
 * @param label_name name of the label of which we want to check address field value
 * @return address field value of a given label name
 */
int address_field_labels_table(labels_table_item *head_table, char *label_name){
    labels_table_item *temp_item = head_table;

    while( strcmp((*temp_item).label_name, label_name) ){ /* i.e. label name of temp_item is different from label name that wanted */
        temp_item = (*temp_item).next;
    }
    return (*temp_item).address;
}

/**
 * returns external field value of a given label name (pay attention: label name must exist in labels table!)
 * @param head_table a pointer to first item in labels table
 * @param label_name name of the label of which we want to check external field value
 * @return external field value of a given label name
 */
flag external_field_labels_table(labels_table_item *head_table, char *label_name){
    labels_table_item *temp_item = head_table;

    while( strcmp((*temp_item).label_name, label_name) ){ /* i.e. label name of temp_item is different from label name that wanted */
        temp_item = (*temp_item).next;
    }
    return (*temp_item).external;
}

/**
 * add a given value to addresses of all labels in labels_table
 * @param head_table a pointer to the head of labels_table
 * @param addition a value to be added to addresses of all labels in labels_table
 */
void add_to_address_labels_table(labels_table_item *head_table, int addition){
    labels_table_item *p = head_table;

    if(head_table != NULL){ /* table is not empty */
        while(p != NULL){
            if( (*p).data == ON )
                (*p).address += addition;
            p = (*p).next;
        }
    }
}

/**
 * Free labels_table
 * @param head_table_p a pointer to the head of labels_table
 */
void free_labels_table(labels_table_item* *head_table_p){
    labels_table_item *p;

    while(*head_table_p){
        p = *head_table_p;
        (*head_table_p) = (*(*head_table_p)).next;
        free(p);
    }
}

/* ##################### now there are functions for handling used_labels_table ##################### */

/**
 * Adds a label to used_labels_table and initialize its fields
 * @param head_table_p a pointer to pointer of the head of used_labels_table
 * @param label_name name of the label to be added
 * @param ic ic value (its position in instruction array, i.e. code segment) of the label to be added
 * @param program_line_copy a string of the program line in which the added label appears
 * @param num_line number of line which makes use in this label as an operand
 * @param external ON if the label which is added appears in a label declaration in a ".extern" line. OFF otherwise.
 * @return ON if label was added successfully to used_labels_table and OFF otherwise
 */
flag add_item_used_labels_table(used_labels_table_item* *head_table_p, char label_name[], int ic, char program_line_copy[], int num_line, flag external){
    used_labels_table_item *p;
    used_labels_table_item *p1;

    p = (used_labels_table_item*)malloc(sizeof(used_labels_table_item));
    if(!p)
        return OFF;

    strcpy((*p).label_name, label_name);
    (*p).ic = ic;
    strcpy( (*p).line, program_line_copy );
    (*p).num_line = num_line;
    (*p).external = external;
    (*p).next = NULL;

    if((*head_table_p) == NULL) {  /* means an empty table */
        (*head_table_p) = p;
        return ON;
    }
    /* if we got here, table is not empty */
    p1 = (*head_table_p);
    while((*p1).next != NULL)
        p1 = (*p1).next;
    (*p1).next = p;
    return ON;
}

/**
 * Scans used_labels_table and for each label it puts label's address in the instructions array (code segment) in
   the label's ic index. In other words, it codes all operands (and parameters) in program which was addressed by label method.
 * @param head_used_labels_table a pointer to the head of used_labels_table
 * @param head_labels_table a pointer to the head of labels_table
 * @param instructions an array which represents the code segment of the program
 * @param err_p a pointer to a variable holding ON if there was an error in program so far, OFF otherwise.
 * @param source_file_name the name of source file (without ".as" postfix)
 */
void handling_used_labels_table(used_labels_table_item *head_used_labels_table, labels_table_item *head_labels_table, int instructions[], flag *err_p, char *source_file_name){
    used_labels_table_item *p = head_used_labels_table;
    int address;
    int aer_field; /* will hold aer (absolute/external/relocatable) field value in decimal */

    if(head_used_labels_table != NULL) {
        while (p != NULL){
            if( search_labels_table( head_labels_table, (*p).label_name) ){
                if( external_field_labels_table(head_labels_table, (*p).label_name) ){
                    (*p).external = ON; /* for future creating of ".ext" file */
                    aer_field = 1; /* decimal value of external bits (01 in binary) */
                }
                else
                    aer_field = 2; /* decimal value of relocatable bits (10 in binary) */
                address = address_field_labels_table(head_labels_table, (*p).label_name);
                address = ( (address<<2) + aer_field );
                instructions[(*p).ic] = address;
            }
            else{ /* the label in used_label_table doesn't exist in labels_table  */
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: using a label which has not been defined in program or declared as external\n", source_file_name, (*p).num_line, (*p).line);
                /* we don't return but continue going through the table in order to find more errors in other program lines */
            }
            p = (*p).next;
        }
    }
}

/**
 * create ".ext" file by scanning used_labels_table and searching (by external field ) for all labels used as external in program
 * @param head_table a pointer to the head of used_labels_table
 * @param full_file_name name of source file (with ".ext" postfix)
 * @return ON if file was created successfully. OFF otherwise.
 */
flag create_extern_file_from_used_labels_table(used_labels_table_item *head_table, char *full_file_name){
    used_labels_table_item *p = head_table;
    FILE *file_p;
    flag first_time = ON; /* will turn OFF when we meet the first label in used_labels_table with extern sign */

    while (p != NULL) {
        if( (*p).external == ON ){
            if(first_time == ON){
                file_p = fopen(full_file_name, "w");
                if (!file_p) {
                    printf("Failing in creating %s\n", full_file_name);
                    return OFF;
                }
                first_time = OFF;
            }
            fprintf(file_p, "%s\t\t%d\n", (*p).label_name, ((*p).ic + INITIAL_ADDRESS) );
        }
        p = (*p).next;
    }
    if(first_time == OFF) /* it means there was at least one used label with extern sign, so a file was opened and need to be closed now */
        fclose(file_p);
    return ON; /* file was created successfully or that there was no need to create .ext file since there was no used label with extern sign */
}

/**
 * Free used_labels_table
 * @param head_table_p a pointer to pointer of the head of used_labels_table
 */
void free_used_labels_table(used_labels_table_item* *head_table_p){
    used_labels_table_item *p;

    while(*head_table_p){
        p = *head_table_p;
        (*head_table_p) = (*(*head_table_p)).next;
        free(p);
    }
}

/* ##################### now there are functions for handling entry_labels_table ##################### */
/**
 * checks if a given label name exist in entry_labels_table or not
 * @param head_table a pointer to the head of entry_labels_table
 * @param name_to_search the label name to be searched
 * @return ON if label name exists in entry_labels_table and OFF otherwise
 */
flag search_entry_labels_table(entry_labels_table_item *head_table, char name_to_search[]){
    entry_labels_table_item *temp_item = head_table;

    if(!head_table)  /* which means empty table */
        return OFF;
    while((*temp_item).next != NULL){
        if( !strcmp((*temp_item).label_name, name_to_search) ) /*true means label_name is equal to name_to_search*/
            return ON;
        temp_item = (*temp_item).next;
    }
    if( !strcmp((*temp_item).label_name, name_to_search) ) /*if we got here, only last item was not checked yet*/
        return ON;
    return OFF;
}

/**
 * Adds a label to entry_labels_table and initialize its fields
 * @param head_table_p a pointer to pointer of the head of entry_labels_table
 * @param label_name name of the label to be added
 * @param program_line_copy a string of the program line in which the added label appears
 * @param num_line number of line which contains the .entry instruction with this label
 * @param address the address of where the label is defined in program (not always is already known when
          adding the label, so it is updated later in "handling_entry_labels_table function").
 * @return ON if label was added successfully to used_labels_table and OFF otherwise
 */
flag add_item_entry_labels_table(entry_labels_table_item* *head_table_p, char label_name[], char program_line_copy[], int num_line, int address){
    entry_labels_table_item *p;
    entry_labels_table_item *p1;

    p = (entry_labels_table_item*)malloc(sizeof(entry_labels_table_item));
    if(!p)
        return OFF;

    strcpy((*p).label_name, label_name);
    strcpy( (*p).line, program_line_copy );
    (*p).num_line = num_line;
    (*p).address = address;
    (*p).next = NULL;

    if((*head_table_p) == NULL) {  /* means an empty table */
        (*head_table_p) = p;
        return ON;
    }
    /* if we got here, table is not empty */
    p1 = (*head_table_p);
    while((*p1).next != NULL)
        p1 = (*p1).next;
    (*p1).next = p;
    return ON;
}

/**
* Scans entry_labels_table and for each label it checks that it is defined in program and take the address of
  label definition from labels_table and puts it in the address field in entry_labels_table.
 * @param head_used_labels_table a pointer to the head of entry_labels_table
 * @param head_labels_table a pointer to the head of labels_table
 * @param err_p a pointer to a variable holding ON if there was an error in program so far, OFF otherwise.
 * @param source_file_name the name of source file (without ".as" postfix)
 */
void handling_entry_labels_table(entry_labels_table_item *head_entry_labels_table, labels_table_item *head_labels_table, flag *err_p, char *source_file_name){
    entry_labels_table_item *p = head_entry_labels_table;

    if(head_entry_labels_table != NULL){
        while(p != NULL){
            if( search_labels_table(head_labels_table, (*p).label_name) )
                (*p).address = address_field_labels_table(head_labels_table, (*p).label_name); /* for future creating of ".ent" file */
            else { /* the label in entry_labels_table doesn't exist in labels_table  */
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: declaring label as entry without defining it in program\n", source_file_name, (*p).num_line, (*p).line);
                /* we don't return but continue going through the table in order to find more errors in other program lines */
            }
            p = (*p).next;
        }
    }
}

/**
 * create ".ent" file by scanning entry_labels_table and printing all labels with their addressess values.
 * @param head_table a pointer to the head of entry_labels_table
 * @param full_file_name name of source file (with ".ent" postfix)
 * @return ON if file was created successfully. OFF otherwise.
 */
flag create_entry_file_from_entry_labels_table(entry_labels_table_item *head_table, char *full_file_name){
    entry_labels_table_item *p = head_table;
    FILE *file_p;

    if(head_table != NULL) {
        file_p = fopen(full_file_name, "w");
        if (!file_p) {
            printf("Failing in creating %s\n", full_file_name);
            return OFF;
        }
        while (p != NULL) {
            fprintf(file_p, "%s\t\t%d\n", (*p).label_name, (*p).address);
            p = (*p).next;
        }
        fclose(file_p);
    }
    return ON; /* file was created successfully or that there was no need to create the file since entry_labels_table is empty */
}

/**
 * Free entry_labels_table
 * @param head_table_p a pointer to pointer of the head of entry_labels_table
 */
void free_entry_labels_table(entry_labels_table_item* *head_table_p){
    entry_labels_table_item *p;

    while(*head_table_p){
        p = *head_table_p;
        (*head_table_p) = (*(*head_table_p)).next;
        free(p);
    }
}

/* ##################### now there are functions for handling macros_table ##################### */
/**
 * checks if a given macro name exist in macros_table or not
 * @param head_table a pointer to the head of macros_table
 * @param name_to_search name of macro to be searched
 * @return ON if macro name exists in macros_table. OFF otherwise.
 */
macros_table_item* search_macros_table(macros_table_item *head_table, char name_to_search[]){
    macros_table_item *temp_item = head_table;

    if(!head_table)  /* which means empty table */
        return NULL;
    while((*temp_item).next != NULL){
        if( !strcmp((*temp_item).macro_name, name_to_search) ) /*true means macro_name is equal to name_to_search*/
            return temp_item;
        temp_item = (*temp_item).next;
    }
    if( !strcmp((*temp_item).macro_name, name_to_search) ) /*if we got here, only last item was not checked yet*/
        return temp_item;
    return NULL;
}

/**
 * calculate number of lines in program inside the body of a given macro name
 * @param item a pointer to a macro in macros_table
 * @return number of lines inside macro's body in program
 */
int size_macros_table(macros_table_item *item){
    return (*item).size;
}

/**
 * returns a string containing the lines in program inside the body of a given macro name
 * @param item a pointer to a macro in macros_table
 * @return a string containing the lines inside macro's body in program
 */
char* content_macros_table(macros_table_item *item){
    return (*item).content;
}

/**
 *
 * @param head_table_p a pointer to pointer of the head of macros_table
 * @param macro_name the name of macro to be added
 * @param content a string of the program lines inside the macro body
 * @param size the number of program lines inside the macro body
 * @return a pointer to the added macro in the macros_table. Null if adding was failed.
 */
macros_table_item* add_item_macros_table( macros_table_item* *head_table_p, char macro_name[], char *content, int size){
    macros_table_item *p;
    macros_table_item *p1;

    p = (macros_table_item*)malloc(sizeof(macros_table_item));
    if(!p)
        return NULL;

    strcpy((*p).macro_name, macro_name);
    (*p).content = content;
    (*p).size = size;
    (*p).next = NULL;

    if((*head_table_p) == NULL) {  /* means an empty table */
        (*head_table_p) = p;
        return p;
    }
    /* if we got here, table is not empty */
    p1 = (*head_table_p);
    while((*p1).next != NULL)
        p1 = (*p1).next;
    (*p1).next = p;
    return p;
}

/**
 * Free macros_table
 * @param head_table_p a pointer to pointer of the head of macros_table
 */
void free_macros_table(macros_table_item* *head_table_p) {
    macros_table_item *p;

    while (*head_table_p) {
        p = *head_table_p;
        (*head_table_p) = (*(*head_table_p)).next;
        free((*p).content);  /* content is a pointer to dynamic memory, hence should be free */
        free(p);
    }
}
