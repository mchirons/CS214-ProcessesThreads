#ifndef compressR_LOLS_h
#define compressR_LOLS_h

#include <stdio.h>

typedef struct Piece {
	int index;
	int size; 
} Piece;

char * compress(char *);
char * intToDecASCII(int x);
Piece * getPieces(int length, int parts);
void saveToFile(char * part, int partnum, const char * filename);
#endif /* Compress_h */
