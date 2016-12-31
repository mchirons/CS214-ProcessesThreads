#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include "compressT_LOLS.h"


/*
This program compresses a file of characters by dividing the file
into a number of pieces given as a command line argument and then
starts a worker thread to do the LOLS compression. This main method
is responsible for delegating the work to its threads.
*/

int totalParts;

int main(int argc, const char **argv){
    
    char * string;
    void *(*tMain)(void*);
    tMain = &threadMain;
    
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
    
   	//printf("size: %d\n", (int)size);
   	//read file into array and make it a string
    string = (char*)malloc(sizeof(char)* (size));
    if (string){
        fread(string, sizeof(char), size, file);
    }
    string[size - 1] = '\0';
    if (strlen(string) == 0){
    	printf("Empty test file\n");
    	exit(EXIT_FAILURE);
    }
    if (parts > strlen(string)) {
        printf("PARTS ARGUMENT TOO LARGE!\n");
        exit(EXIT_FAILURE);
    }
    //printf("original string: %s\n", string);
    //printf("string length: %d\n", (int)strlen(string));
    
    //get pieces
    Piece * pieces = getPieces(strlen(string) , parts);
    int i;
    //int j;
    
    //fork for every part
    char * filename = (char*)malloc(sizeof(char)*(strlen(argv[1])+1));
    pthread_t ids[totalParts]; // array of ids for pthread_join
    strcpy(filename, argv[1]);
    Args * arguments = NULL; //struct passed to each thread containing arguments
    //printf("totalParts Before: %d\n", totalParts);
    for (i = 0; i < totalParts ; i++){ //creats a new thread for how ever many parts desired and sets the structs arguments correctly
        arguments = (Args*)malloc(sizeof(Args));
        arguments->filePath = filename;
        arguments->index = intToDecASCII(pieces[i].index);
        arguments->size = intToDecASCII(pieces[i].size);
        arguments->number = intToDecASCII(i);
        
        pthread_create(&ids[i],NULL,tMain,(void*)arguments);
        
    }
    for (i = 0; i<totalParts; i++) { //joins threads
        pthread_join(ids[i],NULL);
    }	
    //printf("totalParts After: %d\n", totalParts);
    fclose(file);
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

/*
This method is run for each created thread. It opens the file and reads
from the specified index and range given as arguments and then compresses
the part and saves the compressed information in a file.
*/

void* threadMain(void *argv1){
    char * string;
    //char ** argv = *argv1;
    Args * argv = argv1;
    int index = atoi(argv->index);
    int size = atoi(argv->size);
    int partnum = atoi(argv->number);
    FILE * file;
    //open file
   	file = fopen(argv->filePath, "r");
   	//get parts
    
   	//find length of file
   	fseek(file, index, 0);
    
   	//printf("size: %d\n", (int)size);
    
   	//read file into array and make it a string
    string = (char*)malloc(sizeof(char)* (size + 1));
    if (string){
        fread(string, sizeof(char), size+1, file);
    }
    string[size] = '\0';
    //printf("\nuncompressed part: %s\n", string);
    //printf("string length: %d\n", (int)strlen(string));
    //printf("compressed part: %s\n", compress(string));
    
    saveToFile(compress(string), partnum, argv->filePath);
    fclose(file);
    return NULL;
}

/*
Saves a compressed part to a file with the correct part number
appended to the original filename.
*/

void saveToFile(char * part, int partnum, const char * filename){
    char * newName = NULL;
    char * numString = intToDecASCII(partnum);
    int length;
    
    if (partnum == 0){
        newName = (char*)malloc(sizeof(char)*(strlen(filename)+6));
        strcpy(newName, filename); //creates the appropraite filename for the output files
        length = strlen(newName);
        newName[length - 4] = '_';
        newName[length] = '_';
        newName[length + 1] = 'L';
        newName[length + 2] = 'O';
        newName[length + 3] = 'L';
        newName[length + 4] = 'S';
        newName[length + 5] = '\0';
    }
    else{ //creates the appropraite filename for the output files
        int numlength = strlen(numString);
        newName = (char*)malloc(sizeof(char)*(strlen(filename) + 6 + numlength));
        strcpy(newName, filename);
        length = strlen(newName);
        newName[length - 4] = '_';
        newName[length] = '_';
        newName[length + 1] = 'L';
        newName[length + 2] = 'O';
        newName[length + 3] = 'L';
        newName[length + 4] = 'S';
        length = length + 5;
        int i;
        int index = 0;
        for (i = 0; i < numlength; i++, index++){
            newName[length + i] = numString[index];
        }
        newName[length + i] = '\0';
    }
    //printf("saveFile: %s\n", newName);
    FILE * saveFile = fopen(newName, "w+");
    if (fputs(part, saveFile) > 0){
        //printf("file write successful\n");
    }
    else {
        printf("file write unsuccessful\n");
    }
    fclose(saveFile);
    
}


/*
Takes in a char * and uses Length of Long Sequence encoding to compress the string. 
Returns the compressed string. 

Non alphabetic characters are ignored and are not included in the compressed file 
in any form. 

It iterates over the given string and counts the consecutive identical characters 
and creates the compressed string. When a non-alphabetic character is encountered, 
the loop skips over it but the count does not reset, allowing consecutive characters 
separated by non-alphabetic characters to be counted correctly. 

Non-alphabetic characters were not an issue in our function, as they were previously 
treated the same as alphabetic characters. We wrote the function to compress any character of any type given in the string, and it would handle it properly. But since we were told to skip non-alphabetic characters, we had to modify it to do so
*/
char * compress(char * str){
    int i = 0;
    int j = i+1;
    char * countStr;
    char c=' ';
    char * uncomp, * comp;
    int count = 1;
    int len;
    int g = 0;
    uncomp = (char*)malloc(sizeof(char)*(strlen(str)+1));//mallocs space for copy of     uncompressed string
    comp = (char*)malloc(sizeof(char)*(strlen(str)+1));//mallocs return string for compressed string
    strcpy(uncomp, str);
    uncomp[strlen(str)] = '\0';
    for (i=0; i<strlen(str);) {
        //printf("in compress loop\n");
        c=uncomp[i];
        //if statement for only alphabetic characters
        if (!isalpha(c)) {  //checks is alphabetic
            i++;
            continue;
        }
        
        count = 1;
        j=i+1;
        len = strlen(comp);
        while ((uncomp[j]==c || !isalpha(uncomp[j])) && j < strlen(str)){
            if (!isalpha(uncomp[j])) { //if not alphabetic loop continues but count does not reset so consecutive characters divided by non alpha characters are read properly
                i++;
                j++;
                continue;
            }
            count++;
            j++;
            i++;
        }
        if (count == 1) { // if count is 1 only adds the one character
            comp[len]=c;
            comp[len+1]='\0';
        }else if (count == 2) { //if count is 2 adds two of the character to the end string
            comp[len]=c;
            comp[len+1]=c;
            comp[len+2]='\0';
        }else{ //if count is greater than 2 adds the value of count to the string then the character
            countStr = intToDecASCII(count);
            while (g<strlen(countStr)){
            	comp[strlen(comp)+1]='\0';
            	comp[strlen(comp)]=countStr[g];
            	if(g==strlen(countStr)-1){
            		comp[strlen(comp)+1]='\0';
            		comp[strlen(comp)]=c;
            	}
            	g++;
            }
            g=0;
            //comp[len]=(char)(count+'0');
            //comp[len+1] = c;
            //comp[len+2]='\0';
        }
        i++;
    }
    //printf("COMP: %s\nEND\n", comp);
    return comp;
}
