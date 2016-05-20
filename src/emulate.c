#include <stdlib.h>
#include <stdint.h> 
#include <stdio.h> 
#include <assert.h>

int main(int argc, char **argv) {
    assert(argc == 2); 

    // The maximum capacity of the ARM machine memory is 64KB
    const int MAX_SIZE = 65536;
    
    // The maximum number of entries in the main memory 
    const int MAX_ENTRIES = MAX_SIZE / sizeof(int);     

    // Reserving space for main memory 
    uint32_t *mainMem = calloc(MAX_ENTRIES, sizeof(int)); 
     
    // Declaring the binary file  
    FILE *binFile; 
     
    // Defining the binary file with fopen 
    binFile = fopen(argv[1], "rb");
    
    if (binFile) {    
    fread(mainMem, sizeof(int), MAX_ENTRIES, binFile); 
    } else {
    perror("File not found!");
    }

    return 0;    
}
