#ifndef TABLES_H
#define TABLES_H

#include "general.h"
#include "assembler.h"

typedef struct labels_table_item{
    char label_name[MAX_LABEL+1];  /* +1 for '\0' */
    int address;
    flag command; /* will hold ON constant if the label defines a command line and OFF otherwise. */
    flag data;    /* will hold ON constant if the label defines a .data/.string line and OFF otherwise. */
    flag external; /* will hold ON constant if the label is declared as extern and OFF otherwise. */
    flag entry;    /* this field is only for "code perfection" to have here all kinds of fields, but actually,
                      every time we encounter a ".entry" line, we will insert the label declared as entry to
                      entry_labels_table and not to labels_table. Entry_labels_table is useful for pass 2 */
    struct labels_table_item *next;
} labels_table_item;

typedef struct used_labels_table_item{
    char label_name[MAX_LABEL+1];  /* +1 for '\0' */
    int ic; /* index of the cell in instruction array that supposed to contain the encoding of the operand
               in the program which makes use in this label */
    char line[MAX_LINE];  /* a copy of the program line which makes use in this label as an operand */
    int num_line; /* number of line which makes use in this label as an operand */
    flag external;  /* will hold ON constant if the label which is used as an operand is defined as .extern
                       in the program */
    struct used_labels_table_item *next;
} used_labels_table_item;

typedef struct entry_labels_table_item{
    char label_name[MAX_LABEL+1];  /* +1 for '\0' */
    char line[MAX_LINE]; /* a copy of the program line which contains the .entry instruction with this label */
    int num_line; /* number of line which contains the .entry instruction with this label */
    int address;
    struct entry_labels_table_item *next;
} entry_labels_table_item;

typedef struct macros_table_item{
    char macro_name[MAX_LINE]; /* macro size is not limited. Max size of line will be enough, even with '\0'. */
    char *content; /* will point to a dynamic allocated memory containing all program lines inside
                                the macro. */
    int size;  /* number of lines inside macro body */
    struct macros_table_item *next;
} macros_table_item;

/* Now, declarations of functions regarding tables: */

flag search_labels_table(labels_table_item *head_table, char name_to_search[]);

flag add_item_labels_table(labels_table_item* *head_table_p, char label_name[], int address, flag command, flag data, flag external, flag entry);

int address_field_labels_table(labels_table_item *head_table, char *label_name);

flag external_field_labels_table(labels_table_item *head_table, char *label_name);

void add_to_address_labels_table(labels_table_item *head_table, int addition);

void free_labels_table(labels_table_item* *head_table_p);

flag add_item_used_labels_table(used_labels_table_item* *head_table_p, char label_name[], int ic, char program_line_copy[], int num_line, flag external);

void handling_used_labels_table(used_labels_table_item *head_used_labels_table, labels_table_item *head_labels_table, int instructions[], flag *err_p, char *source_file_name);

flag create_extern_file_from_used_labels_table(used_labels_table_item *head_table, char *full_file_name);

void free_used_labels_table(used_labels_table_item* *head_table_p);

flag search_entry_labels_table(entry_labels_table_item *head_table, char name_to_search[]);

flag add_item_entry_labels_table(entry_labels_table_item* *head_table_p, char label_name[], char program_line_copy[], int num_line, int address);

void handling_entry_labels_table(entry_labels_table_item *head_entry_labels_table, labels_table_item *head_labels_table, flag *err_p, char *source_file_name);

flag create_entry_file_from_entry_labels_table(entry_labels_table_item *head_table, char *full_file_name);

void free_entry_labels_table(entry_labels_table_item* *head_table_p);

macros_table_item* search_macros_table(macros_table_item *head_table, char name_to_search[]);

int size_macros_table(macros_table_item *item);

char* content_macros_table(macros_table_item *item);

macros_table_item* add_item_macros_table( macros_table_item* *head_table_p, char macro_name[], char *content, int size);

void free_macros_table(macros_table_item* *head_table_p);

#endif
