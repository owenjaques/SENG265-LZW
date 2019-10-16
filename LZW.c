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

void initDic(){
	int i;
	for(i = 0; i < 256; i++){
		dict[i][0] = 1;
		dict[i][1] = i;
	}
}

/*****************************************************************************/
/* encode() performs the Lempel Ziv Welch compression from the algorithm in  */
/* the assignment specification. The strings in the dictionary have to be    */
/* handled carefully since 0 may be a valid character in a string (we can't  */
/* use the standard C string handling functions, since they will interpret   */
/* the 0 as the end of string marker). Again, writing the codes is handled   */
/* by a separate function, just so I don't have to worry about writing 12    */
/* bit numbers inside this algorithm.                                        */
void encode(FILE *in, FILE *out) {
	char w = 0;
	char k;
	for(k = fgetc(in); k != EOF; k = fgetc(in)){
	}
}

/*****************************************************************************/
/* decode() performs the Lempel Ziv Welch decompression from the algorithm   */
/* in the assignment specification.                                          */
void decode(FILE *in, FILE *out) {
	// TODO implement
}


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
	char file_name[256];
	initDic();
	switch(state){
		case ENCODE:
			snprintf(file_name, 256*sizeof(char), "%s.LZW", argv[1]);
			output_file = fopen(file_name, "w");
			encode(input_file, output_file);
			break;
		case DECODE:
			break;
	}

	return 0;
}
