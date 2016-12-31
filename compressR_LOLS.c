#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "compressR_LOLS.h"




/*
This program compresses a file of characters by dividing the file
into a number of pieces given as a command line argument and then
starts a worker process to do the LOLS compression. This main method
is responsible for delegating the work to its child processes.
*/

int totalParts;

int main(int argc, const char **argv){
    
    char * string;
    
    FILE * file;
    int parts = 0;
    if (argc != 3){printf("Incorrect number of arguments.\n"); exit(EXIT_FAILURE);}
    //open file
   	file = fopen(argv[1], "r");
   	
   	if (file == NULL){
   		printf("File open error.");
		exit(EXIT_FAILURE);
	}
   	//get parts
   	parts = atoi(argv[2]);
    if (parts <1) {
        printf("ILLEGAL PARTS ARGUMENT\n");
        exit(EXIT_FAILURE);
    }
   	//find length of file
   	fseek(file, 0L, SEEK_END);
   	long size = ftell(file);
   	rewind(file);
   	//printf("size: %ld\n", size);

   	//printf("size: %d\n", (int)size);

   	//read file into array and make it a string
    string = (char*)malloc(sizeof(char)* (size));
    if (string){
    	fread(string, sizeof(char), size, file);
    }
    if (strlen(string) == 0){
    	printf("Empty test file\n");
    	exit(EXIT_FAILURE);
    }
    string[size - 1] = '\0';
    if (parts > strlen(string)) {
        printf("PARTS ARGUMENT TOO LARGE!\n");
        exit(EXIT_FAILURE);
    }
    //printf("original string: %s\n", string);
    //printf("string length: %d\n", (int)strlen(string));

    //get pieces
    Piece * pieces = getPieces(strlen(string) , parts);
    int i;
    
    //fork for every part
    char * filename = (char*)malloc(sizeof(char)*(strlen(argv[1])+1));
    strcpy(filename, argv[1]);
    char * args[6];
    args[0] = "compressRworker";
    args[1] = filename;
    args[5] = NULL;
    //printf("totalParts: %d\n", totalParts);
    for (i = 0; i < totalParts ; i++){
    	args[2] = intToDecASCII(pieces[i].index);
    	args[3] = intToDecASCII(pieces[i].size);
    	args[4] = intToDecASCII(i);
    	if(!fork()){
    		//printf("\nchild spawned\n");
    		
    		if(execv(args[0], args) < 0){
    			printf("exec() failed\n");
    		}
    		_exit(1);
    		break;

    	}
    }
    //wait for children to finish
    while (1) {
	    int status;
	    pid_t done = wait(&status);
	    if (done == -1) {
	        if (errno == ECHILD) {
	        	break;
	        }
	       
	    }
	}
    wait(NULL);
    fclose(file);
    
    //printf("process time: %d\n", time);
    return 0;
}

/*
This method returns a pointer to a list of Piece structs. The number of
piece structs corresponds to the number of parts that the file should
be broken into for compression. This method calculated what index of the
file each piece begins as well as the length of the pieces.
*/

Piece * getPieces(int length, int parts){ //divides file string into necessary parts
	int i; 
	int remainder = 0;
	int index = 0; 
	int size = 0;
	totalParts = parts;
	Piece * pieces = NULL;
	//printf("length: %d\n", length);
	if (length % parts){
		remainder = length % parts;
	}
	size = length / parts;
	pieces = (Piece*)malloc(sizeof(Piece*) * parts);
	for (i = 0; i < parts; i++){
		pieces[i] = (Piece){.index = 0, .size = size};
	}
	if (remainder){
		for (i = 0; i < remainder; i++){
			pieces[i].size++;
		}
	}
	for (i = 0; i < parts; i++){
		pieces[i].index = index;
		index = index + pieces[i].size;
	}

	return pieces;
}


/*
Takes an integer argument and returns a string representation of
that integer
*/
char* intToDecASCII(int x){
	char *s;
	if(x == 0){
		return "0";
	}
	int y = x;
	int size = 0;
	int isNegative = x < 0;
	while (y /= 10){
		size++;
	}
	size++;
	if (isNegative){ //checks if param is negative and makes positive
		size++;
		x = x * -1;
	}
	s = (char*)malloc(size + 1);
	s[size] = '\0';
	while (size--){
		s[size] = x % 10 + '0';
		x /= 10;
	}
	if (isNegative){ // if the param was negative it adds the negative sign to the output
		s[0] = '-';
	}
	return s;
}
