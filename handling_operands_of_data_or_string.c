#include "general.h"
#include "assembler.h"

/**
 * Checking if data part of .data/.string is valid. If it is, it codes it into data array. Else, prints error.
 * @param storage an array containing only the data part of an .data/.string instruction (with maybe space/tab
                  chars in the beginning) including a '\n' char in the end.
 * @param classification holds DATA constant in case of ".data" instruction, and STRING constant in case of
                         ".string" instruction (these constants are defined in assembler_both_passes.h)
 * @param data array containing the data code of the source program
 * @param dc_p a pointer to variable (dc) holding the index of the next free cell in data array.
 * @param err_p a pointer to a variable (err) holding ON constant if we discovered an error so far in the
                source program and OFF constant if we didn't.
 * @param program_line_copy a copy of current program line
 * @param source_file_name name of source file (without ".as" postfix) 
 * @param num_line the number of current program line
 */
void validate_and_encode_operands_of_data_or_string(char storage[], class classification, int data[], int *dc_p, flag *err_p, char program_line_copy[], char *source_file_name, int num_line){
    int num;
    int k = 0, m = 0; /* just 2 running indexes */
    int c;  /* will hold a character */
    flag in_digit = OFF;  /* holding ON constant if we are in the middle of scanning a number, or we are after
                             scanning number but still didn't scan comma. Get OFF constant every time we scan a
                             comma. Initiallized to OFF constant. This variable will help discovering errors in
                             data syntax while scanning data. Constants are defined in assembler_both_passes.h */

    switch(classification) {
        /* case DATA is for .data instruction */
        case DATA:
            /* first of all check there is at least 1 number in the data */
            num = 0;
            while ((c=storage[k++]) != '\n' ) {
                if (isdigit(c)){
                    num++;
                    break;
                }
            }
            if(num == 0){
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: there is no number in data\n",source_file_name, num_line, program_line_copy);
                return;
            }

            /* after we made sure there is at least 1 number in data, now we are going to check
               that the data field of the instruction is valid */
            k = 0;
            while((c=storage[k++]) != '\n' ){
                /* pay attention: k was incremented by 1 and now storage[k] points to char after(!) the char in c */
                switch(c){
                    case ' ':
                    case '\t':
                        if(in_digit == OFF){
                            if(storage[k] != ',')
                                break;  /* break from switch, which means continue for next while iteration */
                            else{
                                (*err_p) = ON;
                                if( (&storage[k])==strchr(storage,',') ) /* storage[k] contains ','. If this else condition is true (inclduing in_digit=OFF, it means the ',' is the first char in data field */
                                    printf("File: %s.am, line %d: \"%s\", error: data starts with a comma instead of a number\n",source_file_name, num_line, program_line_copy);
                                else /* the ',' isn't the first char in data field */
                                    printf("File: %s.am, line %d: \"%s\", error: there are 2 commas without a number in between\n",source_file_name, num_line, program_line_copy);
                                return;
                            }
                        }
                        else{    /* c is space/tab  and  in_digit is ON */
                            if((storage[k] == ' ') || (storage[k] == '\t') || (storage[k] == '\n') || (storage[k] == ',') )
                                break;  /* break from switch, which means continue for next while iteration */
                            else{
                                (*err_p) = ON;
                                printf("File: %s.am, line %d: \"%s\", error: only space, tab, '\\n' and comma are allowed after a number\n",source_file_name, num_line, program_line_copy);
                                return;
                            }
                        }
                    case ',':
                        if(in_digit == OFF){
                            (*err_p) = ON;
                            if( (&storage[k-1])==strchr(storage,',') ) /* storage[k-1] contains ','. If this if condition is true (including is_digit=OFF), it means the ',' is the first char in data field */
                                printf("File: %s.am, line %d: \"%s\", error: data starts with a comma instead of a number\n",source_file_name, num_line, program_line_copy);
                            else /* the ',' is not the first char in data field */
                                printf("File: %s.am, line %d: \"%s\", error: there are 2 commas without a number in between\n",source_file_name, num_line, program_line_copy);
                            return;
                        }
                        else{  /* c is comma  and in_digit is ON */
                            if((storage[k] == ',') || (storage[k] == '\n') ){
                                (*err_p) = ON;
                                printf("File: %s.am, line %d: \"%s\", error: there is no number after comma\n",source_file_name, num_line, program_line_copy);
                                return;
                            }
                            else{
                                in_digit = OFF;
                                break;  /* break from switch, which means continue for next while iteration */
                            }
                        }
                    case '+':
                    case '-':
                        if( isdigit(storage[k]) )  /* next char is a digit */
                            break;  /* break from switch, which means continue for next while iteration */
                        else{
                            (*err_p) = ON;
                            printf("File: %s.am, line %d: \"%s\", error: only a digit is allowed after '+'/'-' character\n",source_file_name, num_line, program_line_copy);
                            return;
                        }
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        if((storage[k] == '+') || (storage[k] == '-') ){
                            (*err_p) = ON;
                            printf("File: %s.am, line %d: \"%s\", error: '+'/'-' character is not allowed after a number\n",source_file_name, num_line, program_line_copy);
                            return;
                        }
                        else{
                            in_digit = ON;
                            break;  /* break from switch, which means continue for next while iteration */
                        }
                    default:  /* c is not a digit, comma, '+', '-', space or tab. */
                        (*err_p) = ON;
                        printf("File: %s.am, line %d: \"%s\", error: there is invalid character in data arguments\n",source_file_name, num_line, program_line_copy);
                        return;
                } /* internal switch */
            } /* while */

            if(in_digit == OFF){
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: there is no number after the last comma\n",source_file_name, num_line, program_line_copy);
                return;
            }

            /* if we got here, data is valid and we just need to encode it into data array */
            k = 0;
            while((c=storage[k++]) != '\n' ){
                if( c!='+' && c!='-' && (!isdigit(c)) )
                    continue;
                while( c=='+' || c=='-' || isdigit(c) ){
                    storage[m] = c;
                    m++;
                    c = storage[k++];
                }
                storage[m] = '\0';
                data[*dc_p] = atoi(storage);
                (*dc_p) = (*dc_p) + 1;
                m = 0;
                if(c == '\n') /* this check is necessary because if we got out from internal while loop since c
                              is '\n', then we have to notice that k points to character after c and not to c.
                              Hence, without this check, external while loop will skip termination character. */
                    break;
            }
            return;

            /* case STRING is for .string instruction */
        case STRING:
            while(((c=storage[k++]) == ' ') || c == '\t' )
            {}
            if(c=='\n'){
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: there is no string\n",source_file_name, num_line, program_line_copy);
                return;
            }
            if(c!='"'){
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: string should start with quotation marks\n",source_file_name, num_line, program_line_copy);
                return;
            }
            /* now c is '"' (opening quotation mark of a string) and storage[k] is the char after c in the string */
            while( (isprint((c=storage[k++])) || c==' ' || c=='\t') && c!='"' ){ /* assuming char in a string can be any printable char or space/tab char except for '"' char */
                storage[m] = c;
                m++;
            }
            storage[m] = '\0';
            /* now c is the first invalid char to be within a string */
            if(c!='"' && c!='\n'){
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: there is invalid character in string\n",source_file_name, num_line, program_line_copy);
                return;
            }
            if(c=='\n'){
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: string should end with quotation marks\n",source_file_name, num_line, program_line_copy);
                return;
            }
            /* if we got here, c is '"' and string doesn't contain invalid chars. */
            /* let's check string is not empty */
            if(m == 0){
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: string can not be empty\n",source_file_name, num_line, program_line_copy);
                return;
            }
            /* let's check now that there is no invalid (non-white char) after closing '"' of the string */
            while(((c=storage[k++]) == ' ') || c == '\t' )
            {}
            if(c!='\n'){
                (*err_p) = ON;
                printf("File: %s.am, line %d: \"%s\", error: only one string is allowed and no (non-white) char is allowed after it\n",source_file_name, num_line, program_line_copy);
                return;
            }
            /* if we got here then string is valid, and now we just need to encode it into data array */
            k = 0;
            while((c=storage[k++]) != '\0' ){
                data[*dc_p] = c;
                (*dc_p) = (*dc_p) + 1;
            }
            data[*dc_p] = '\0';
            (*dc_p) = (*dc_p) + 1;
            return;
    /* the cases below are never reached. I wrote it just because a compiler warning */       
    case EMPTY_LINE_OR_COMMENT:
    case EXTERN:
    case ENTRY:
    case COMMAND:
    case UNDEFINED:
        break;
    }  /* external switch */
}  /* validate_and_encode_operands_of_data_or_string */
