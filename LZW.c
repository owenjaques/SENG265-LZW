#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define DICTSIZE 4096 /* allow 4096 entries in the dict  */
#define ENTRYSIZE 30

#define ENCODE 1
#define DECODE 2

int read12(FILE *infil);
int write12(FILE *outfil, int int12);
void strip_lzw_ext(char *fname);
void flush12(FILE *outfil);
void initDic();
int inDic(unsigned char entry[ENTRYSIZE], int current_dic_index);
void encode(FILE *in, FILE *out);
void decode(FILE *in, FILE *out);


unsigned char dict[DICTSIZE][ENTRYSIZE];
/* of 30 chars max; the first byte */
/* is string length; index 0xFFF   */
/* will be reserved for padding    */
/* the last byte (if necessary)    */

//adds the initial 255 ASCII chars to dictionary
void initDic(){
	int i;
	for(i = 0; i < 256; i++){
		dict[i][0] = 1;
		dict[i][1] = i;
	}
}

//a helper function to check whether or not an entry is in the dictionary
//params: entry[ENTRYSIZE] (unsigned char array)
//	current_dic_index (int) - very important so that the function knows how much of the dictionary to check
//returns: -1 if the entry cannot be found else the index of the dictionary where it is
int inDic(unsigned char entry[ENTRYSIZE], int current_dic_index){
	int i, j;
	for(i = 0; i < current_dic_index; i++){
		int this_in_dic = 0;//indicates if the entry is at this index in the dictionary
		for(j = 0; j <= entry[0]; j++){
			if(entry[j] == dict[i][j])
				this_in_dic = 1;
			else {
				this_in_dic = 0;
				break;
			}
		}
		if(this_in_dic)
			//if it has found the entry return the index it is at 
			return i;
	}
	//return -1 if the entry is not found
	return -1;
}

/*****************************************************************************/
/* encode() performs the Lempel Ziv Welch compression from the algorithm in  */
/* the assignment specification. The strings in the dictionary have to be    */
/* handled carefully since 0 may be a valid character in a string (we can't  */
/* use the standard C string handling functions, since they will interpret   */
/* the 0 as the end of string marker). Again, writing the codes is handled   */
/* by a separate function, just so I don't have to worry about writing 12    */
/* bit numbers inside this algorithm.                                        */
void encode(FILE *in, FILE *out){
	unsigned char w[ENTRYSIZE] = {0};//an unsigned char array stored in the same format as the dict entries with length at index 0
	unsigned char k;
	unsigned char wk[ENTRYSIZE] = {};//similar to w but with k, length stored at index 0
	int current_w_index;//used for writing the dict code of w to the file
	int current_dic_index = 256;//used to keep track of the next dictionary index to put a new wk at
	
	k = fgetc(in);
	while(!feof(in)){
		int i;
		int in_dic = 0;//a flag to indicate if something was in the dict for going in between the if statements without having to call inDic each time
		int dont_add = 0;//a rarely used flag to indicate if w reached the entry size and to not add wk to dict

		//if wk would be to large to hold in the dictionary skip to outputting w
		if(w[0] < ENTRYSIZE - 2){
			//creates wk from w and k
			for(i = 0; i <= w[0]; i++)
				wk[i] = w[i];
			wk[i] = k;
			wk[0]++;
			//sees if wk is in the dictionary and sets the current w index accordingly
			int temp = inDic(wk, current_dic_index);
			if(temp != -1){
				in_dic = 1;
				current_w_index = temp;
			}
		}
		else
			dont_add = 1;//wk has reached the entry size dont add to dict and skip to outputting

		if(in_dic){
			//sets w to wk
			for(i = 0; i <= w[0]; i++)
				w[i] = wk[i];

		}
		else {
			write12(out, current_w_index);
			if(!dont_add){
				//adds wk to dictionary if not full
				if(current_dic_index < DICTSIZE - 1){
					dict[current_dic_index][0] = wk[0];
					for(i = 1; i <= wk[0]; i++){
						dict[current_dic_index][i] = wk[i];
					}
					current_dic_index++;
				}
			}
			w[0] = 1;
			w[1] = k;
			current_w_index = k;
		}
		k = fgetc(in);
	}
	write12(out, current_w_index);
	flush12(out);
}

/*****************************************************************************/
/* decode() performs the Lempel Ziv Welch decompression from the algorithm   */
/* in the assignment specification.                                          */
void decode(FILE *in, FILE *out) {
	unsigned char *w;//points to a dictionary entry
	int k = read12(in);
	//this will only go off if the file contained no 12 bit numbers
	if(k == -1){
		printf("Error Invalid Format\n");
		exit(3);
	}
	int current_dic_index = 256;
	fputc(dict[k][1], out);
	w = dict[k];
	k = read12(in);
	//runs until the end of file is reached or the padding bit is detected
	while(!feof(in) && k != DICTSIZE - 1){
		//if k is greater than the current_dic_index it indicates the file has been corrupted
		if(k > current_dic_index){
			printf("Error Invalid Format\n");
			exit(3);
		}
		//checks if k is in the dictionary
		if(dict[k][0]){
			int i;
			//outputs dict[k] to file
			for(i = 1; i <= dict[k][0]; i++)
				fputc(dict[k][i], out);
			//quickly makes wk from w + k[1] if it would be less than the entry size
			if(w[0] < ENTRYSIZE -2){
				unsigned char wk[ENTRYSIZE];
				for(i = 0; i <= w[0]; i++)
					wk[i] = w[i];
				wk[i] = dict[k][1];
				wk[0]++;
				//adds wk to the dictionary if it is not already there and there is room
				if(inDic(wk, current_dic_index) == -1 && current_dic_index < DICTSIZE - 1){
					for(i = 0; i <= wk[0]; i++)
						dict[current_dic_index][i] = wk[i];
					current_dic_index++;
				}
			}
		}
		else {
			//creates ww (w + w[1]) if it would be less than the entry size 
			if(w[0] < ENTRYSIZE - 2){
				int i;
				unsigned char ww[ENTRYSIZE];
				for(i = 0; i <= w[0]; i++)
					ww[i] = w[i];
				ww[i] = w[1];
				ww[0]++;
				//add ww to the dict if it does not already exit and there is room
				if(inDic(ww, current_dic_index) == -1){
					if(current_dic_index < DICTSIZE - 1){
						for(i = 0; i <= ww[0]; i++)
							dict[current_dic_index][i] = ww[i];
						current_dic_index++;
					}
				}
				//outputs ww to file regardless if there was room in the dict for it
				for(i = 1; i <= ww[0]; i++)
					fputc(ww[i], out);
			}
		}
		w = dict[k];
		k = read12(in);
	}
}

/*****************************************************************************/
/* read12() handles the complexities of reading 12 bit numbers from a file.  */
/* It is the simple counterpart of write12(). Like write12(), read12() uses  */
/* static variables. The function reads two 12 bit numbers at a time, but    */
/* only returns one of them. It stores the second in a static variable to be */
/* returned the next time read12() is called.                                */
int read12(FILE *infil)
{
 static int number1 = -1, number2 = -1;
 unsigned char hi8, lo4hi4, lo8;
 int retval;

 if(number2 != -1)                        /* there is a stored number from   */
    {                                     /* last call to read12() so just   */
     retval = number2;                    /* return the number without doing */
     number2 = -1;                        /* any reading                     */
    }
 else                                     /* if there is no number stored    */
    {
     if(fread(&hi8, 1, 1, infil) != 1)    /* read three bytes (2 12 bit nums)*/
        return(-1);
     if(fread(&lo4hi4, 1, 1, infil) != 1)
        return(-1);
     if(fread(&lo8, 1, 1, infil) != 1)
        return(-1);

     number1 = hi8 * 0x10;                /* move hi8 4 bits left            */
     number1 = number1 + (lo4hi4 / 0x10); /* add hi 4 bits of middle byte    */

     number2 = (lo4hi4 % 0x10) * 0x0100;  /* move lo 4 bits of middle byte   */
                                          /* 8 bits to the left              */
     number2 = number2 + lo8;             /* add lo byte                     */

     retval = number1;
    }

 return(retval);
}

/*****************************************************************************/
/* write12() handles the complexities of writing 12 bit numbers to file so I */
/* don't have to mess up the LZW algorithm. It uses "static" variables. In a */
/* C function, if a variable is declared static, it remembers its value from */
/* one call to the next. You could use global variables to do the same thing */
/* but it wouldn't be quite as clean. Here's how the function works: it has  */
/* two static integers: number1 and number2 which are set to -1 if they do   */
/* not contain a number waiting to be written. When the function is called   */
/* with an integer to write, if there are no numbers already waiting to be   */
/* written, it simply stores the number in number1 and returns. If there is  */
/* a number waiting to be written, the function writes out the number that   */
/* is waiting and the new number as two 12 bit numbers (3 bytes total).      */
int write12(FILE *outfil, int int12)
{
 static int number1 = -1, number2 = -1;
 unsigned char hi8, lo4hi4, lo8;
 unsigned long bignum;

 if(number1 == -1)                         /* no numbers waiting             */
    {
     number1 = int12;                      /* save the number for next time  */
     return(0);                            /* actually wrote 0 bytes         */
    }

 if(int12 == -1)                           /* flush the last number and put  */
    number2 = 0x0FFF;                      /* padding at end                 */
 else
    number2 = int12;

 bignum = number1 * 0x1000;                /* move number1 12 bits left      */
 bignum = bignum + number2;                /* put number2 in lower 12 bits   */

 hi8 = (unsigned char) (bignum / 0x10000);                     /* bits 16-23 */
 lo4hi4 = (unsigned char) ((bignum % 0x10000) / 0x0100);       /* bits  8-15 */
 lo8 = (unsigned char) (bignum % 0x0100);                      /* bits  0-7  */

 fwrite(&hi8, 1, 1, outfil);               /* write the bytes one at a time  */
 fwrite(&lo4hi4, 1, 1, outfil);
 fwrite(&lo8, 1, 1, outfil);

 number1 = -1;                             /* no bytes waiting any more      */
 number2 = -1;

 return(3);                                /* wrote 3 bytes                  */
}

/** Write out the remaining partial codes */
void flush12(FILE *outfil)
{
 write12(outfil, -1);                      /* -1 tells write12() to write    */
}                                          /* the number in waiting          */

/** Remove the ".LZW" extension from a filename */
void strip_lzw_ext(char *fname)
{
    char *end = fname + strlen(fname);

    while (end > fname && *end != '.' && *end != '\\' && *end != '/') {
        --end;
    }
    if ((end > fname && *end == '.') &&
        (*(end - 1) != '\\' && *(end - 1) != '/')) {
        *end = '\0';
    }
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

	FILE* input_file = fopen(argv[1], "rb");

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
			output_file = fopen(file_name, "wb");
			encode(input_file, output_file);
			break;
		case DECODE:
			strip_lzw_ext(argv[1]);
			output_file = fopen(argv[1], "wb");
			decode(input_file, output_file);
			break;
	}
	fclose(output_file);
	fclose(input_file);

	return 0;
}
