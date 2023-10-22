assembler: main.o pre_processor.o assembler_both_passes.o handling_labels_of_extern_or_entry.o handling_line_without_operands.o handling_operands_of_command_line.o handling_operands_of_data_or_string.o tables.o
	gcc -g -Wall -ansi -pedantic main.o pre_processor.o assembler_both_passes.o handling_labels_of_extern_or_entry.o handling_line_without_operands.o handling_operands_of_command_line.o handling_operands_of_data_or_string.o tables.o -o assembler
main.o: main.c general.h
	gcc -c -g -Wall -ansi -pedantic main.c -o main.o
pre_processor.o: pre_processor.c general.h tables.h assembler.h
	gcc -c -g -Wall -ansi -pedantic pre_processor.c -o pre_processor.o
assembler_both_passes.o: assembler_both_passes.c general.h tables.h assembler.h
	gcc -c -g -Wall -ansi -pedantic assembler_both_passes.c -o assembler_both_passes.o
handling_labels_of_extern_or_entry.o: handling_labels_of_extern_or_entry.c general.h tables.h assembler.h
	gcc -c -g -Wall -ansi -pedantic handling_labels_of_extern_or_entry.c -o handling_labels_of_extern_or_entry.o
handling_line_without_operands.o: handling_line_without_operands.c general.h assembler.h
	gcc -c -g -Wall -ansi -pedantic handling_line_without_operands.c -o handling_line_without_operands.o
handling_operands_of_command_line.o: handling_operands_of_command_line.c general.h tables.h assembler.h
	gcc -c -g -Wall -ansi -pedantic handling_operands_of_command_line.c -o handling_operands_of_command_line.o
handling_operands_of_data_or_string.o: handling_operands_of_data_or_string.c general.h assembler.h
	gcc -c -g -Wall -ansi -pedantic handling_operands_of_data_or_string.c -o handling_operands_of_data_or_string.o
tables.o: tables.c general.h tables.h assembler.h
	gcc -c -g -Wall -ansi -pedantic tables.c -o tables.o
	 				
						
