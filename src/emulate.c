#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

uint32_t createMask(uint32_t bot, uint32_t top);
int branch(uint32_t instr);
int checkCond(uint32_t cond, uint32_t CPSRpntr);
int checkZ(uint32_t CPSRpntr);
int equalityNV(uint32_t CPSRpntr);

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

int checkCond(uint32_t cond, uint32_t CPSRpntr) {
    /* This function is used to check the condition of the instruction.
    It calls helper functions checkZ and equalityNV to return a value
    of 1 if the condition is met, or 0 if the condition is not met. */
    switch(cond) {
        case 0  : return checkZ(CPSRpntr); 
        case 1  : return !checkZ(CPSRpntr);
        case 10 : return equalityNV(CPSRpntr);
        case 11 : return !equalityNV(CPSRpntr);
        case 12 : return (!checkZ(CPSRpntr) && equalityNV(CPSRpntr));
        case 13 : return (checkZ(CPSRpntr) || !equalityNV(CPSRpntr));
        default : assert(cond == 14); return 1;
    } 
}

int checkZ(uint32_t CPSRpntr) {
    /* This function checks the 30th bit of the CPSR register (the Z
       bit) to see if it is set. Returns 1 if Z is set. Returns 0 if
       it is clear. */
    return getBits(30, 30, CPSRpntr);
}

int equalityNV(uint32_t CPSRpntr) {
    /* This function checks the 31st bit of the CPSR register (the N
       bit) and compares it with the 28th bit (the V bit). Returns 1 of
       N = C. Returns 0 if N != C */
    int N = getBits(31, 31, CPSRpntr);
    int V = getBits(28, 28, CPSRpntr);
    return N == V;
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
