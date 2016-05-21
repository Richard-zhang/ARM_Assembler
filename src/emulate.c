#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

uint32_t createMask(uint32_t bot, uint32_t top);
int branch(uint32_t instr);
int checkCond(uint32_t cond, uint32_t CPSRpntr);
int checkZ(uint32_t CPSRpntr);
int equalityNV(uint32_t CPSRpntr);

struct state {
    uint32_t decoded;
    uint32_t fetched; 
};  


/* Selects the bits from a range when given a leftmost and a rightmost bit, 
and returns the selected bits as a 32-bit int */ 
uint32_t getBits(int leftmost, int rightmost, uint32_t num) {
    assert(leftmost >= rightmost && leftmost < numsize(uint32_t) * CHAR_BIT);
    int range = leftmost - rightmost;
    uint32_t mask = (sizeof(char) << (range + sizeof(char)) - sizeof(char); 
    mask <<= rightmost;
    num &= mask;
    num >>= rightmost;
    return num;   
}

/*Switches the endianness of a 32-bit int by swapping the positions of each
byte */ 
uint32_t switchEndian(uint32_t num) {  
    uint32_t a = (num & 0x000000ff)  << 24; 
    uint32_t b = (num & 0x0000ff00)  << 8;
    uint32_t c = (num & 0x00ff0000)  >> 8;
    uint32_t d = (num & 0xff000000)  >> 24;
    return a | b | c | d;
}

// returns the value of a register given the number of the register 
uint32_t accessReg(uint32_t *regFile, int reg){
    return regFile[reg];     
}

int main(int argc, char **argv) {
    assert(argc == 2); 
    
    // The number of the CPSR register 
    const int CPSRREG = 16; 

    // The maximum capacity of the ARM machine memory is 64KB
    const int MAX_SIZE = 65536;
    
    // The maximum number of entries in the main memory 
    const int MAX_ENTRIES = MAX_SIZE / sizeof(int);     

    // Allocating space for the main memory 
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
   
    // Closes and disassociates the binary file  
    fclose(binFile); 
    
    // The number of registers that an ARM system contains 
    const int NUM_REG = 17;  
 
    // Allocating space for the registers 
    uint32_t *regFile = calloc(NUM_REG, sizeof(int));    
    
    // Stores the current state of the ARM machine  
    struct state *ARMState = malloc(sizeof(struct state)); 
    assert(ARMState != NULL);     
    
    ARMState->decoded = 0;
    ARMState->fetched = 0; 
    
    /* The pipeline begins its execution. The first instruction is fetched and 
    then decoded.*/
    
    ARMState->decoded = mainMem[0];       
   
    

    // infinite loop 
    for(;;) {
  
      // Checks if 
      if(ARMState->decoded == 0){
            break;
        } 

    /* Checks the condition code of the instruction, which specifies whether
    the instruction will be executed or not */ 
    if (checkCond(cond, *(regFile + CPSRREG)) == 1){
        // execute 
    }
 
    ARMState->decoded = ARMState->fetched; 
    ARMState->fetched = mainMem[*regFile]; 
    
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
    return (N & V);
}
	
int branch(uint32_t instr) {
    /* branch is a helper function that contains an 24 bit 2s complement
       offset that will be sign extended and added to "PC" */
    
    // extracts offset from the instruction
    int32_t offset = getBits(23, 0, instr);
    
    /* offset is shifted left 2 bits to account for the PC being 8 bytes
       ahead of the instruction that is being executed */  
    int shift = 2;
    offset <<= shift;
   // uint32_t mostSigBitIndex = 25;
   // mask = createMask(mostSigBitIndex, mostSigBitIndex);
    
    mostSigBitIndex = getBits(25, 25, offset);

    // check most significant bit to see if a sign extension is needed
    if(mostSigBitIndex == 1){
        uint32_t endOfInstr = 31;
        uint32_t signExtension = createMask(endOfInstr, mostSigBitIndex);
        offset = signExtension | mask;
    }
    // most significant bit of 0 will not need a sign extension
    return offset;
}

uint32_t createMask(uint32_t bot, uint32_t top) {
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
