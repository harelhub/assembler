#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#define DATA_INSTRUCTION ".data"
#define STRING_INSTRUCTION ".string"
#define ENTRY_INSTRUCTION ".entry"
#define EXTERN_INSTRUCTION ".extern"

#define GARBAGE_VALUE -999

/* these constants will help for classifying each program line in assembler_both_passes function */
typedef enum{EMPTY_LINE_OR_COMMENT, DATA, STRING, EXTERN, ENTRY, COMMAND, UNDEFINED} class;

/* these constants will help to represent a current binary status, if it is A or B, if it is yes or no, etc. */
typedef enum{OFF, ON} flag;

/* these constants will help to represent how a particular label appear in a current analyzed program
   line, as "label:" definition, as ".entry label" or as ".extern label" */
typedef enum{LABEL_DEFINITION, LABEL_EXTERN, LABEL_ENTRY} add_label_status;

/* these constants will help to represent the addressing method of a particular operand (or parameter).
   ADDRESSING_0 - ADDRESSING_3 are the 4 valid addressing methods of assembly, NO_OPERAND says there is no
   operand at all, WRONG_ADDRESSING says this is an invalid addressing method (either because of invalid
   syntax or because it isn't allowed for this operand with the current command in the analyzed line) */
typedef enum{ADDRESSING_0, ADDRESSING_1, ADDRESSING_2, ADDRESSING_3, NO_OPERAND, WRONG_ADDRESSING} addressing_value;

/* these constants represent the opcodes values of the various commands in assembly*/
typedef enum{MOV_CODE, CMP_CODE, ADD_CODE, SUB_CODE, NOT_CODE, CLR_CODE, LEA_CODE, INC_CODE,
             DEC_CODE, JMP_CODE, BNE_CODE, RED_CODE, PRN_CODE, JSR_CODE, RTS_CODE, STOP_CODE, NONE} opcode;

/* these constants will help represent an answer to question about if first word in a particular program
   line is a label. NOT_LABEL says it is not a label at all, VALID_LABEL says it is a valid label, INVALID_LABEL
   says it is supposed to be a label but it has a syntax error */
typedef enum{NOT_LABEL, VALID_LABEL, INVALID_LABEL} is_first_word_a_label;

/* will be used as a variable type for the items in commands names table */
typedef struct{
    char *command_name;
    opcode command_code;
    flag src_addressing[4];
    flag des_addressing[4];
} command;


#endif

