#include <stdlib.h>
#include <stdio.h>
#define MAXLINE 32

void endianConvert(char[]);
void swap(char *, char *);

int main(int argc, char **argv) {
    /* reading source file */
    char *inname  = argv[1];
    char *outname = argv[2];
    FILE *inFile;
    FILE *outFile;

    char lineBuffer[MAXLINE];

    inFile    = fopen(inname, "r");
    outFile   = fopen(outname, "wb");

    if (inFile == NULL) {
        exit(EXIT_FAILURE);
    }

    while(fgets(lineBuffer, MAXLINE, inFile)) {
        printf("%s", lineBuffer);
    }

    fclose(outFile);
    fclose(inFile);
    return 0;
}

//dataProcessingConverter
void endianConvert(char *ip) {
    swap(ip, ip + 3);
    swap(ip + 1, ip + 2);
}

void swap(char *first, char *end) {
    char temp = *first;
    *first = *end;
    *end   = temp;
}
