#ifndef compressT_LOLS_h
#define compressT_LOLS_h

#include <stdio.h>

typedef struct Piece {
    int index;
    int size;
} Piece;

typedef struct Args {
    char* filePath;
    char* index;
    char* size;
    char* number;
} Args;

char * compress(char *);
char * intToDecASCII(int x);
Piece * getPieces(int length, int parts);
void saveToFile(char * part, int partnum, const char * filename);
void * threadMain(void*);
void saveToFile(char * , int , const char * );
char * compress(char * );
#endif /* compressT_LOLS_h */
