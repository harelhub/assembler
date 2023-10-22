#include "general.h"
#include "tables.h"
#include "assembler.h"

void handling_command_operands_phase1(char storage[], opcode command_opcode, char program_line_copy[], flag *err_p, int instructions[], int *ic_p, int dc, char *source_file_name, command commands_table[], used_labels_table_item* *head_used_labels_table_p, flag *allocate_err_p, int num_line);
void handling_command_operands_phase2(opcode command_opcode, char src[], char des[], addressing_value *src_type_p, addressing_value *des_type_p, char param1[], char param2[], addressing_value *param1_type_p, addressing_value *param2_type_p);
flag handling_command_operands_phase3(opcode command_opcode, addressing_value src_type, addressing_value des_type, addressing_value param1_type, addressing_value param2_type, command commands_table[], flag *err_p, char *source_file_name, char program_line_copy[], int num_line);
void handling_command_operands_phase4(opcode command_opcode, char src[], char des[], char param1[], char param2[], addressing_value src_type, addressing_value des_type, addressing_value param1_type, addressing_value param2_type, int *ic_p, int instructions[], int dc, char program_line_copy[], used_labels_table_item* *head_used_labels_table_p, flag *allocate_err_p, int num_line);
void handling_command_operands_phase5(char src[], char des[], char param1[], char param2[], addressing_value src_type_copy, addressing_value des_type, addressing_value param1_type, addressing_value param2_type, int *ic_p, int instructions[], int dc, char program_line_copy[], used_labels_table_item* *head_used_labels_table_p, flag *allocate_err_p, int num_line);
flag is_command_with_two_operands(opcode command_opcode);


/**
 * checks validity of operands in command line. If valid, code them into instruction array (code segment).
 * @param storage array containing part of line which comes after command word. Ends with '\n'. Array can start with space/tab chars.
 * @param command_opcode opcode value of the command in the current program line
 * @param program_line_copy a copy of current program line for error prints
 * @param err_p a pointer to variable holding ON if we found an error in spanned file so far, OFF otherwise
 * @param instructions array which represent the code segemnt of the source program
 * @param ic_p a pointer to variable holding the index of next free cell in instructions array (code segment)
 * @param dc a variable holding the index of next free cell in data array (data segment)
 * @param source_file_name the name of source file (without ".as" postfix)
 * @param commands_table a table containing details about the various assembly commands
 * @param head_used_labels_table_p a pointer to the pointer of the head of used_labels_table
 * @param allocate_err_p a pointer to a variable holding ON if we encountered a memory allocation error, OFF otherwise
 * @param num_line the number of current program line
 */
void handling_command_operands_phase1(char storage[], opcode command_opcode, char program_line_copy[], flag *err_p, int instructions[], int *ic_p, int dc, char *source_file_name, command commands_table[], used_labels_table_item* *head_used_labels_table_p, flag *allocate_err_p, int num_line){
    char src[MAX_LINE], des[MAX_LINE]; /*will contain src/dest operands respectively. Will be without white chars
                                 before. Will end with '\0'. If there is no src operand then src[0] will be '\0'*/
    addressing_value src_type, des_type; /* will contain the addressing value of src/des operand respectively
                                           (see addressing values in assembler_both_passes.h) */
    char param1[MAX_LINE], param2[MAX_LINE]; /* will contain parameter 1 and 2 respectively in case we have jump
                                      addressing method. Will be without white chars before. Will end with '\0' */
    addressing_value param1_type, param2_type; /*will contain the addressing value of parameter 1 and 2
                                                respectively (see addressing values in assembler_both_passes.h)*/
    int m=0, k=0;
    int c;

    while( ((c=storage[k++])==' ') || c=='\t' ){
    }
    if(c == '\n'){ /* there are no operands at all */
        if( (command_opcode!=RTS_CODE) && (command_opcode!=STOP_CODE) ){
            (*err_p) = ON;
            printf("File: %s.am, line %d: \"%s\", error: there are no operands in this command line\n",source_file_name, num_line, program_line_copy);
            return;
        }
        else { /* this is a RTS or STOP command line */
            instructions[(*ic_p)] = (command_opcode << 6);
            (*ic_p) = (*ic_p) + 1;
            return;
        }
    }
    /* if we got here, there is at least one non-white char in operands field. As we strarted above to deal with
       a case of RTS/STOP command with no operand, it is convenient to deal here with the complement case there
       is a char in operands field and by this to finish deal with RTS/STOP command case */
    if(command_opcode==RTS_CODE || command_opcode==STOP_CODE){
        (*err_p) = ON;
        printf("File: %s.am, line %d: \"%s\", error: this command should not have operands\n",source_file_name, num_line, program_line_copy);
        return;
    }
    /* if we got here, this is not a RTS/STOP command */
    k--; /*after this decrement storage[k] is c, i.e. the first char in storage array which is not space/tab char or '\n'*/
    if( command_opcode!=JMP_CODE && command_opcode!=BNE_CODE && command_opcode!=JSR_CODE ){ /*command is not a jumping command*/
        while( (c=storage[k++])!=' ' && c!='\t' && c!=',' && c!='\n' ){
            src[m] = c;
            m++;
        }
        src[m] = '\0';
        if(m==0){ /* as mentioned in comment above, c wasn't space/tab/'\n' char. Hence, if there was only 1 iteration, c was a ',' */
            (*err_p) = ON;
            printf("File: %s.am, line %d: \"%s\", error: there shouldn't be a comma after a command word\n",source_file_name, num_line, program_line_copy);
            return;
        }
        k--;
        while( (c=storage[k++])==' ' || c=='\t' ){
        }
        if(c=='\n'){ /* there is only one operand so we put it in des array and put '\0' in src, because in assembly language a command can get only des operand but no command can get only src operand */
            strcpy(des, src);
            src[0] = '\0';
        }
        else{
            if(c!=','){
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: there is no comma between operands\n",source_file_name, num_line, program_line_copy);
                return;
            }
            else{  /* c is a ',' and it is the first char after src operand which is not a white char. storage[k] is the char after c. */
                while( (c=storage[k++])==' ' || c=='\t' ){
                }
                if(c=='\n'){
                    (*err_p) = ON;
                    printf("File: %s.am, line %d: \"%s\", error: there is no destination operand after comma\n",source_file_name, num_line, program_line_copy);
                    return;
                }
                k--; /* after this decrement storage[k] will be c char (and not the char after c) */
                m = 0;
                while( (c=storage[k++])!=' ' && c!='\t' && c!='\n' && c!=',' ){
                    des[m] = c;
                    m++;
                }
                des[m] = '\0';
                if(c==','){
                    (*err_p) = ON;
                    printf("File: %s.am, line %d: \"%s\", error: there is more than one comma\n",source_file_name, num_line, program_line_copy);
                    return;
                }
                if(c==' ' || c=='\t'){
                    while( (c=storage[k++])==' ' || c=='\t' ){
                    }
                    if(c==','){
                        (*err_p) = ON;
                        printf("File: %s.am, line %d: \"%s\", error: there is more than one comma\n",source_file_name, num_line, program_line_copy);
                        return;
                    }
                    if(c!='\n'){
                        (*err_p) = ON;
                        printf("File: %s.am, line %d: \"%s\", error: too many operands\n",source_file_name, num_line, program_line_copy);
                        return;
                    }
                }
            }
        }
    }
    else { /* command is a jumping command, i.e. command_opcode is JMP_CODE/BNE_CODE/JSR_CODE */
        /* let's remember c now is not a space/tab/'\n' char */
        if(c==','){
            (*err_p) = ON;
            printf("File: %s.am, line %d: \"%s\", error: there shouldn't be a comma after a command word\n",source_file_name, num_line,  program_line_copy);
            return;
        }
        while ((c = storage[k++]) != ' ' && c != '\t' && c != '\n') {
            des[m] = c;
            m++;
        }
        des[m] = '\0';
        src[0] = '\0';
        if(c==' ' || c=='\t'){
            while( (c=storage[k++])==' ' || c=='\t' ){
            }
            if(c!='\n'){
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: jumping command should have only des operand and its addressing format should be one of: 1).\"label\" 2).\"label(param1,param2)\" while params are \"#number\"/\"label\"/\"ri\"(i is between 0-7) 3).\"ri\" (i is between 1 and 7).\n",source_file_name, num_line, program_line_copy);
                return;
            }
        }
    }
    /* if we got here, then src and des operands have been extracted successfully to src and des
       arrays (whether there are both src and des operands in command line or only des operand). Moreover,
       we know that this is not a RTS or STOP command since we have already dealt with such case */

    handling_command_operands_phase2(command_opcode, src, des, &src_type, &des_type, param1, param2, &param1_type, &param2_type);
    
    if( command_opcode!=JMP_CODE && command_opcode!=BNE_CODE && command_opcode!=JSR_CODE ){
        if(src_type==WRONG_ADDRESSING || des_type==WRONG_ADDRESSING){
            if(src_type==WRONG_ADDRESSING)
                printf("File: %s.am, line %d: \"%s\", error: invalid source operand\n",source_file_name, num_line, program_line_copy);
            else
                printf("File: %s.am, line %d: \"%s\", error: invalid destination operand\n",source_file_name, num_line, program_line_copy);
            (*err_p) = ON;
            return;
        }
    }
    else{ /* this is a jumping command */
        if(des_type==WRONG_ADDRESSING){ /* there is only des operand in jumping command and it contains the parameters too in its field */
            printf("File: %s.am, line %d: \"%s\", error: jumping command should have only des operand and its addressing format should be one of: 1).\"label\" 2).\"label(param1,param2)\" while params are \"#number\"/\"label\"/\"ri\"(i is between 0-7) 3).\"ri\" (i is between 1 and 7).\n",source_file_name, num_line, program_line_copy);
            (*err_p) = ON;
            return;
        }
    }
    /* if we got here, src and des operands are in src and des arrays and their addressing values are in src_type
       and des_type. If this is a jumping command with a jump addressing method then the parameters are in
       param1 and param2 and their addressing values are in param1_type and param2_type. Let's check now if the
       addressing values of the operands are allowed with the specific current command */

    if( handling_command_operands_phase3(command_opcode, src_type, des_type, param1_type, param2_type, commands_table, err_p, source_file_name, program_line_copy, num_line) == OFF)
        return;
    
    /* if we got here, the operands of the command are valid and matches to the command. Now we just have to
       code the command with its operands into the instructions array (code image) */
    handling_command_operands_phase4(command_opcode, src, des, param1, param2, src_type, des_type, param1_type, param2_type, ic_p, instructions, dc, program_line_copy, head_used_labels_table_p, allocate_err_p, num_line);

}


/**
 * calculate src/des operand's addressing (method) value and put it into src_type/des_type respectively. If this
   is a jumping command with jumping addressing method, it puts both of the parameters into param1/param2
   respectively and their addressing (method) value into param1_type/param2_type respectively. (see addressing values in assembler_both_passes.h).
 * @param command_opcode opcode value of the command in the current program line (see opcode values in assembler_both_passes.h)
 * @param src contain src operand without white chars before. Ends with '\0'. If there is no src operand then src[0] will be '\0'
 * @param des contain dest operand without white chars before. End with '\0'.
 * @param src_type_p a pointer to variable holding the addressing value of src operand (see addressing values in assembler_both_passes.h)
 * @param des_type_p a pointer to variable holding the addressing value of dest operand (see addressing values in assembler_both_passes.h)
 * @param param1 contain parameter 1 in case we have jump addressing method. Without white chars before. End with '\0'
 * @param param2 contain parameter 2 in case we have jump addressing method. Without white chars before. End with '\0'
 * @param param1_type_p a pointer to variable holding the addressing value of parameter 1 in case we have jump addressing method (see addressing values in assembler_both_passes.h)
 * @param param2_type_p a pointer to variable holding the addressing value of parameter 2 in case we have jump addressing method (see addressing values in assembler_both_passes.h)
 */
void handling_command_operands_phase2(opcode command_opcode, char src[], char des[], addressing_value *src_type_p, addressing_value *des_type_p, char param1[], char param2[], addressing_value *param1_type_p, addressing_value *param2_type_p){
    int j, k=0, m=0;
    int c;
    char *p1;
    addressing_value *p2;

    if( command_opcode!=JMP_CODE && command_opcode!=BNE_CODE && command_opcode!=JSR_CODE ) { /* "main if" - deals with a non-jumping command */
        p1 = src;
        p2 = src_type_p;
        for(j=1; j<=2; j++){ /*2 iterations. First for calculating src_type and second for calculating des_type*/
            if(p1[0]=='\0') /* i.e. operand is empty */
                (*p2) = NO_OPERAND;
            else{
                if( p1[0]=='r' && (p1[1]>='0' && p1[1]<='7') && p1[2]=='\0' ) /* operand is a name of a register */
                    (*p2) = ADDRESSING_3;
                else{
                    while( isupper((c=p1[k++])) || islower(c) || isdigit(c) ){
                    } /* empty loop */
                    if( c=='\0' && (isupper(p1[0]) || islower(p1[0])) ) /* operand is a label name (since chars are only letters/digits and first char is a letter) */
                        (*p2) = ADDRESSING_1;
                    else{ /* if we got here, operand is not empty, but it is also not a register/label */
                        k = 2;
                        while( isdigit((c=p1[k++])) ){
                        } /* empty loop */
                        if(p1[0]=='#' && (isdigit(p1[1]) || p1[1]=='+' || p1[1]=='-') && c=='\0') /* operand is a constant (since first char is '#', second char is '+'/'-'/digit and then only digits) */
                            (*p2) = ADDRESSING_0;
                        else /* if we got here, operand is not empty but it is also not a register/label/constant. Therefore, it has error
                                in its syntax (we consider here ADDRESSING_2 too as an error, since "main if" deals with non-jumping command) */
                            (*p2) = WRONG_ADDRESSING;
                    }
                }
            }
            /* now some preparations for next iteration of for loop */
            k = 0;
            p1 = des;
            p2 = des_type_p;
        } /* closing of for loop */
    } /* closing of "main if" */

    else{ /* "main else" - deals with jumping command (jmp/bne/jsr) */
        /* there is no need to check if src operand is empty because in handling_command_operands_phase1 we
           have automatically put '\0' in src[0] in cases of jumping command */
        (*src_type_p) = NO_OPERAND;
        /* there is no need to check if des operand is empty because we have already checked in handling_command_operands_phase1
           that there is at least one operand and in such case we have put it into des array */
        if( des[0]=='r' && (des[1]>='0' && des[1]<='7') && des[2]=='\0' ) /* des operand is a name of a register */
            (*des_type_p) = ADDRESSING_3;
        else{ /* des operand is not a register */
            while( isupper((c=des[k++])) || islower(c) || isdigit(c) ){
            } /* empty loop */
            if(c=='\0' && (isupper(des[0]) || islower(des[0])) ) /* des operand is a label name (since chars are only letters/digits and first char is a letter) */
                (*des_type_p) = ADDRESSING_1;
            else{ /* des operand is not empty/register/label. A case of jump addressing method (ADDRESSING_2) remains to be checked */
                if( ( (!isupper(des[0])) && (!islower(des[0])) ) || c!='(' ) /* des operand is not of jump addressing method, since there first char is not a letter or that after label there is no '(' */
                    (*des_type_p) = WRONG_ADDRESSING;
                else{ /* des operand does starts with a valid label name and then '(' as needed in jump addressing method */
                    while( (c=des[k++])!=',' && c!='\0' ){
                        param1[m] = c;
                        m++;
                    }
                    param1[m] = '\0';
                    if(c=='\0') /* des operand is not of jump addressing method, since there is no comma after first parameter */
                        (*des_type_p) = WRONG_ADDRESSING;
                    else{ /* there is a comma after first parameter */
                        m = 0;
                        while( (c=des[k++])!=')' && c!='\0' ){
                            param2[m] = c;
                            m++;
                        }
                        param2[m] = '\0';
                        if(c == '\0') /* des operand is not of jump addressing method, since there is no ')' after second parameter */
                            (*des_type_p) = WRONG_ADDRESSING;
                        else{ /* there is a ')' after second parameter */
                            if(des[k] != '\0') /* des operand is not of jump addressing method, since this function gets des array with '\0' right after des operand, so if des[k]!='\0' it means there is invalid char after ')' */
                                (*des_type_p) = WRONG_ADDRESSING;
                            else{ /* if we got here, des operand has a pattern of "label(param1,param2)". Now we call recursively
                                     to this function in order to calculate addressing value of param1/param2. This works because
                                     1).both param1/param2 starts immediately with the parameter without white chars before and ends
                                     with '\0' 2).addressing value of param1/param2 can't be jump addressing method. Due to these 2
                                     facts, call to this function with any non-jumping command opcode and delivering param1/param2/param1_type_p/param2_type_p
                                     as parameters instead of src/des/src_type_p/des_type_p, will go into the "main if" which deals with
                                     non-jumping command and will calculate successfully param1/param2 addressing values and put
                                     them into param1_type_p/param2_type_p respectively */
                                handling_command_operands_phase2(MOV_CODE, param1, param2, param1_type_p, param2_type_p, NULL, NULL, NULL, NULL);
                                if( (*param1_type_p)==NO_OPERAND || (*param1_type_p)==WRONG_ADDRESSING || (*param2_type_p)==NO_OPERAND || (*param2_type_p)==WRONG_ADDRESSING ) /* missing or invalid param1/param2 in a jump addressing method */
                                    (*des_type_p) = WRONG_ADDRESSING;
                                else /* all checks have gone successfully and this is jump addressing method for sure!   */
                                    (*des_type_p) = ADDRESSING_2;
                            }
                        }
                    }
                }
            }
        }
    } /* closing of "main else" */


    }

/**
 * check if the addressing methods of the operands (and parameters) are allowed with the current command
 * @param command_opcode opcode value of the command in case the current line is a command line (see opcode values in assembler_both_passes.h)
 * @param src_type the addressing value of src operand (see addressing values in assembler_both_passes.h)
 * @param des_type the addressing value of des operand (see addressing values in assembler_both_passes.h)
 * @param param1_type the addressing value of parameter 1 in case we have jump addressing method (see addressing values in assembler_both_passes.h)
 * @param param2_type the addressing value of parameter 2 in case we have jump addressing method (see addressing values in assembler_both_passes.h)
 * @param commands_table a table with information about types of assembly commands and their valid addressing methods
 * @param err_p a pointer to a flag which gets ON if an error has been discovered so far in the program and OFF otherwise (ON and OFF are defined in assembler_both_passes.h)
 * @param source_file_name the name of source file (without ".as" postfix)
 * @param program_line_copy a copy of current program line
 * @param num_line the number of current program line
 * @return ON if there is a match between the addressing method of operands (and parameters) and the command, and OFF otherwise
 */
flag handling_command_operands_phase3(opcode command_opcode, addressing_value src_type, addressing_value des_type, addressing_value param1_type, addressing_value param2_type, command commands_table[], flag *err_p, char *source_file_name, char program_line_copy[], int num_line){
    int j;

    if(src_type == NO_OPERAND){
        for(j=0; j<4; j++){
            if(commands_table[command_opcode].src_addressing[j] == ON){ /* this command has at least one allowed addressing method for src operand, hence must get src operand*/
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: this command must have both src and des operands\n",source_file_name, num_line, program_line_copy);
                return OFF;
            }
        }
    }
    else {
        if( !is_command_with_two_operands(command_opcode)){ /* remember in phase3 function we don't deal with stop/rts commands. Hence, command can get src+des operands or only des operand and here we check it. */
            (*err_p) = ON;
            printf("File: %s.am, line %d: \"%s\", error: this command should not have a source operand\n",
                   source_file_name, num_line, program_line_copy);
            return OFF;
        }
        if (commands_table[command_opcode].src_addressing[src_type] == OFF) { /* this addressing method of src operand is not allowed with this command */
            (*err_p) = ON;
            printf("File: %s.am, line %d: \"%s\", error: addressing method of source operand is not allowed with this command\n",
                   source_file_name, num_line, program_line_copy);
            return OFF;
        }
    }
    if(des_type == NO_OPERAND){
        for(j=0; j<4; j++){
            if(commands_table[command_opcode].des_addressing[j] == ON){ /* this command has at least one allowed addressing method for des operand, hence must get des operand*/
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: this command must have both src and des operands\n",source_file_name, num_line, program_line_copy);
                return OFF;
            }
        }
    }
    else {
        if (commands_table[command_opcode].des_addressing[des_type] == OFF) { /* this addressing method of des operand is not allowed with this command */
            (*err_p) = ON;
            printf("File: %s.am, line %d: \"%s\", error: addressing method of destination operand is not allowed with this command\n",
                   source_file_name, num_line, program_line_copy);
            return OFF;
        }
    }
    if( (des_type==ADDRESSING_2) && (param1_type==NO_OPERAND || param2_type==NO_OPERAND) ){ /* this is a jumping command and there is a missing parameter */
        (*err_p) = ON;
        printf("File: %s.am, line %d: \"%s\", error: jumping command with jump addressing method must have exactly 2 parameters\n",
               source_file_name, num_line, program_line_copy);
        return OFF;
    }
    /* all checks were successful so we can return ON */
    return ON;
}



/**
 * codes the command line with its operands into instructions array (code segment)
 * @param command_opcode opcode value of the command in case the current line is a command line (see opcode values in assembler_both_passes.h)
 * @param src contain src operand without white chars before. Ends with '\0'. If there is no src operand then src[0] will be '\0'
 * @param des contain dest operand without white chars before. End with '\0'.
 * @param param1 contain parameter 1 in case we have jump addressing method. Without white chars before. End with '\0'
 * @param param2 contain parameter 2 in case we have jump addressing method. Without white chars before. End with '\0'
 * @param src_type the addressing value of src operand (see addressing values in assembler_both_passes.h)
 * @param des_type the addressing value of des operand (see addressing values in assembler_both_passes.h)
 * @param param1_type the addressing value of parameter 1 in case we have jump addressing method (see addressing values in assembler_both_passes.h)
 * @param param2_type the addressing value of parameter 2 in case we have jump addressing method (see addressing values in assembler_both_passes.h)
 * @param ic_p a pointer to variable holding the index of next free cell in instructions array (code segment)
 * @param instructions array which represent the code segemnt of the source program
 * @param dc a variable holding the index of next free cell in data array (data segment)
 * @param program_line_copy a copy of current program line
 * @param head_used_labels_table_p a pointer to the pointer of the head of used_labels_table
 * @param allocate_err_p a pointer to a variable holding ON if we encountered a memory allocation error, OFF otherwise
 * @param num_line the number of current program line
 */
void handling_command_operands_phase4(opcode command_opcode, char src[], char des[], char param1[], char param2[], addressing_value src_type, addressing_value des_type, addressing_value param1_type, addressing_value param2_type, int *ic_p, int instructions[], int dc, char program_line_copy[], used_labels_table_item* *head_used_labels_table_p, flag *allocate_err_p, int num_line){
    addressing_value src_type_copy;

    src_type_copy = src_type;
    if(src_type == NO_OPERAND)
        src_type = ADDRESSING_0; /* the purpose is that the bits of source operand addressing method will be zeros
                                    in the calculation below in case there is no source operand (no need to do it
                                    also with des_type since there is always des operand, we have already dealt with rts/stop commands in phase1)  */
    if(des_type != ADDRESSING_2) /* i.e. this is not a jumping command (so there are no parameters) */
        instructions[*ic_p] = ( (command_opcode<<6) + (src_type<<4) + (des_type<<2) );
    else /* this is a jumping command (with parameters. We checked this in phase3 that there is no missing parameter, i.e. their addressing method is not NO_OPERAND) */
        instructions[*ic_p] = ( (param1_type<<12) + (param2_type<<10) + (command_opcode<<6) + (des_type<<2) );
    (*ic_p) = (*ic_p) + 1;
    /* we have coded the first info-word of the command line. Now we should code the other info-words */
    handling_command_operands_phase5(src, des, param1, param2, src_type_copy, des_type, param1_type, param2_type, ic_p, instructions, dc, program_line_copy, head_used_labels_table_p, allocate_err_p, num_line);
}

/**
 * codes the command line with its operands into instructions array (code segment)
 * @param src contain src operand without white chars before. Ends with '\0'. If there is no src operand then src[0] will be '\0'
 * @param des contain dest operand without white chars before. End with '\0'.
 * @param param1 contain parameter 1 in case we have jump addressing method. Without white chars before. End with '\0'
 * @param param2 contain parameter 2 in case we have jump addressing method. Without white chars before. End with '\0'
 * @param src_type_copy the addressing value of src operand (see addressing values in assembler_both_passes.h)
 * @param des_type the addressing value of des operand (see addressing values in assembler_both_passes.h)
 * @param param1_type the addressing value of parameter 1 in case we have jump addressing method (see addressing values in assembler_both_passes.h)
 * @param param2_type the addressing value of parameter 2 in case we have jump addressing method (see addressing values in assembler_both_passes.h)
 * @param ic_p a pointer to variable holding the index of next free cell in instructions array (code segment)
 * @param instructions array which represent the code segemnt of the source program
 * @param dc a variable holding the index of next free cell in data array (data segment)
 * @param program_line_copy a copy of current program line
 * @param head_used_labels_table_p a pointer to the pointer of the head of used_labels_table
 * @param allocate_err_p a pointer to a variable holding ON if we encountered a memory allocation error, OFF otherwise
 * @param num_line the number of current program line
 */
void handling_command_operands_phase5(char src[], char des[], char param1[], char param2[], addressing_value src_type_copy, addressing_value des_type, addressing_value param1_type, addressing_value param2_type, int *ic_p, int instructions[], int dc, char program_line_copy[], used_labels_table_item* *head_used_labels_table_p, flag *allocate_err_p, int num_line){
    int k = 0;

    if(des_type != ADDRESSING_2){
        if(src_type_copy == ADDRESSING_0){ /* src operand is an immediate value */
            instructions[*ic_p] = ( atoi(&src[1])<<2 );
            (*ic_p) = (*ic_p) + 1;
        }
        else
            if(src_type_copy == ADDRESSING_1){ /* src operand is a label */
                if( !add_item_used_labels_table(head_used_labels_table_p, src, *ic_p, program_line_copy, num_line, OFF) ){
                    (*allocate_err_p) = ON;
                    return;
                }
                (*ic_p) = (*ic_p) + 1;
            }
            else
                if(src_type_copy == ADDRESSING_3){ /* src operand is a register */
                    instructions[*ic_p] = ( atoi(&src[1])<<8 );
                    (*ic_p) = (*ic_p) + 1;
                }
        /* if we got here, source operand was coded (we didn't deal with a case src_type_copy is NO_OPERAND
           since no coding word is given in this case). Now let's code destinatio operand. */
        if(des_type == ADDRESSING_0){ /* des operand is an immediate value */
            instructions[*ic_p] = ( atoi(&des[1])<<2 );
            (*ic_p) = (*ic_p) + 1;
            return; /* we finished to code src and des operands */
        }
        if(des_type == ADDRESSING_1){ /* des operand is a label */
            if( !add_item_used_labels_table(head_used_labels_table_p, des, *ic_p, program_line_copy, num_line, OFF) ){
                (*allocate_err_p) = ON;
                return;
            }
            (*ic_p) = (*ic_p) + 1;
            return; /* we finished to code src and des operands */
        }
        if(des_type == ADDRESSING_3){ /* des operand is a register */
            if(src_type_copy == ADDRESSING_3){ /* if both src and des operands are registers, they are coded in a single info-word */
                instructions[(*ic_p)-1] = ( instructions[(*ic_p)-1] + (atoi(&des[1])<<2) );
                return; /* we finished to code src and des operands */
            }
            else{ /* src_type_copy is not ADDRESSING_3 */
                instructions[*ic_p] = ( atoi(&des[1])<<2 );
                (*ic_p) = (*ic_p) + 1;
                return; /* we finished to code src and des operands */
            }
        }
        /* There are no more options for des_type since it can't be ADDRESSING_2 as the if condition above
           checked, and it can't be NO_OPERAND since no des operand is an error and we have already dealt with
           errors in operands in earlier phases. Hence, we have finished to code src and des operands */
    }
    else{ /* des_type is ADDRESSING_2 */
        while(des[k++]!='('){
        }
        des[k-1] = '\0';
        if( !add_item_used_labels_table(head_used_labels_table_p, des,*ic_p, program_line_copy, num_line, OFF) ){ /* adding the label we jump to, to used_labels_table */
            (*allocate_err_p) = ON;
            return;
        }
        (*ic_p) = (*ic_p) + 1;
        /* now we should code the parameters in this jump addressing method. For this, we can call "phase5" recursively and this will
        work because when we got here param1_type and param2_type can be only ADDRESSING_0, ADDRESING_1 or ADDRESSING_3 (ADDRESSING_2
        and NO_OPERAND which are invalid for parameters were dealt in earlier phases). These are exactly the same addressing method 
        values that operand in a non-jumping command can get, and since param2_type is not ADDRESSING_2, the recursive call will analyze
        the parameters like it analyze the non-jumping command line operands (we deliver param1/param2/param1_type/param2_type as 
        parameters to src/des/src_type/des_type respectively, NULL as parameter to param1/param2 and a non-ADDRESSING_2 value, let's     say ADDRESSING_0, as parameter to param1_type/param2_type) */
        
        handling_command_operands_phase5(param1, param2, NULL, NULL, param1_type, param2_type, ADDRESSING_0, ADDRESSING_0, ic_p, instructions, dc, program_line_copy, head_used_labels_table_p, allocate_err_p, num_line);
        return;
    }
}

/**
* checks if a given opcode value blongs to a command which should get both src and des operands
* @command_opcode an opcode value of the command to be checked
* @return ON if command should get both src and des operands, OFF otherwise.
*/
flag is_command_with_two_operands(opcode command_opcode){
    if(command_opcode==MOV_CODE || command_opcode==CMP_CODE || command_opcode==ADD_CODE || command_opcode==SUB_CODE || command_opcode==LEA_CODE)
        return ON;
    return OFF;
}
