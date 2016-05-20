#include "stdio.h"
#include "stdlib.h"
#define MAXLEN 1024

void swap(char *first, char *end) {
    char temp = *first;
    *first = *end;
    *end   = temp;
}

void endianConvert(char *ip) {
    swap(ip, ip + 3);
    swap(ip + 1, ip + 2);
}

int main(int argc, char *argv[]) {
    FILE *outfile, *infile;
    outfile = fopen(argv[2], "wb");
    infile  = fopen(argv[1], "rb");
    char buf[MAXLEN];

    int rc;
    //size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
    //size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
    int count = 0;
    while ((rc = fread(buf, sizeof(unsigned char), 4, infile)) != 0) {
        ++count;
        endianConvert(buf);
        fwrite(buf, sizeof(unsigned char), rc, outfile);
    }

    printf("%d\n", count);
    fclose(infile);
    fclose(outfile);

    return 0;
}
