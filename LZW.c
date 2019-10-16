#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int main(int argc, char *argv[]){
	if(argc <= 2 || argv[2][0] != 'e' && argv[2][0] != 'd'){
		printf("Invalid Usage, expected: LZW {input_file} [e|d]\n");
		exit(4);
	}
	
	if(argv[1][0] == '\0' || isspace(argv[1][0])){
		printf("Error: No input file specified!\n");
		exit(1);
	}

	FILE* input_file = fopen(argv[1], "r");

	if(input_file == NULL){
		printf("Read error: file not found or cannot be read\n");
		exit(2);
	}

	FILE* output_file = fopen(NAME_OF_FILE, "w");



	return 0;
}
