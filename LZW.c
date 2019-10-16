#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define TRUE 1
#define FALSE 0

#define DICTSIZE 4096                     /* allow 4096 entries in the dict  */
#define ENTRYSIZE 32

#define ENCODE 1
#define DECODE 2

unsigned char dict[DICTSIZE][ENTRYSIZE];  /* of 30 chars max; the first byte */
                                          /* is string length; index 0xFFF   */
                                          /* will be reserved for padding    */
                                          /* the last byte (if necessary)    */


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
	
	int state = argv[2][0] == 'e' ? ENCODE : DECODE;
	FILE* output_file;
	switch(state){
		case ENCODE:
			printf("case is encode\n");
			char file_name[50];
			snprintf(file_name, sizeof(argv[1]) + 3*sizeof(char), "%s.LZW", argv[1]);
			output_file = fopen(file_name, "w");
			break;
		case DECODE:
			printf("case is decode\n");
			break;
	}

	return 0;
}
