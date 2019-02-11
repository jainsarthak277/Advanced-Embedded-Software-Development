#include <stdio.h>
#include <stdlib.h>
#define FILE_NAME "Genie.txt"

void main()
{
	char interest_char, test_char;
	char* interest_str;
	char* test_string;
	
	printf("Hello\n");
	FILE *file_ptr = fopen(FILE_NAME, "w+");
	printf("Write ONLY ONE character to the file\n");
	interest_char = getc(stdin);
	fputc(interest_char, file_ptr);
	fclose(file_ptr);
	__fpurge(stdin);

	file_ptr = fopen(FILE_NAME, "a");
	interest_str = (char *)malloc(20*sizeof(char));
	printf("Enter a string to append: ");
	fgets(interest_str, 20, stdin);
	fputs(interest_str, file_ptr);
	fflush(file_ptr);
	fclose(file_ptr);

	file_ptr = fopen(FILE_NAME, "r");
	test_char = getc(file_ptr);
	printf("Character 1 of file is: %c\n", test_char);
	test_string = (char *)malloc(20*sizeof(char));
	fgets(test_string, 20, file_ptr);
	printf("%s\n", test_string);
	fclose(file_ptr);
	
	free(interest_str);
	free(test_string);
}
