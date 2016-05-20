#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

uint32_t createMask(uint32_t bot, uint32_t top);
int branch(uint32_t instr);

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
	
int branch(uint32_t instr) {
    /* branch is a helper function that contains an 24 bit 2s complement
       offset that will be sign extended and added to "PC" */
    
    // extracts offset from the instruction
    uint32_t offsetStart = 0;
    uint32_t offsetEnd = 23;
    uint32_t mask = createMask(offsetEnd, offsetStart);

    int32_t offset = mask & instr;
    
    /* offset is shifted left 2 bits to account for the PC being 8 bytes
       ahead of the instruction that is being executed */  
    int shift = 2;
    offset = offset << shift;
    uint32_t mostSigBitIndex = 25;
    mask = createMask(mostSigBitIndex, mostSigBitIndex);

    // check most significant bit to see if a sign extension is needed
    if(mask & offset != 0){
        uint32_t endOfInstr = 31;
        uint32_t signExtension = createMask(endOfInstr, mostSigBitIndex);
        offset = signExtension | mask;
    }
    // most significant bit of 0 will not need a sign extension
    return offset;
}

uint32_t createMask(uint32_t top, uint32_t bot) {
    /* createMask is a helper function that produces a mask (with correct
       offset) */
    assert(bot <= top); 
    uint32_t mask = 0;
    int i;

    for(i = bot; i <= top; i++) {
        mask |= 1 << i;
    }
    return mask;
}    
