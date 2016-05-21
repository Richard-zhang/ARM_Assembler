#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#define CPSR 16
#define PC 15
#define NUM_REG 17 
#define GEN_REG 13 
uint32_t createMask(uint32_t top, uint32_t bot);
int branch(uint32_t instr);
int checkCond(uint32_t cond, uint32_t CPSRpntr);
int checkZ(uint32_t CPSRpntr);
int equalityNV(uint32_t CPSRpntr);
uint32_t multInstr(uint32_t *regFile, uint32_t instr);
uint32_t accMultInstr(uint32_t *regFile, uint32_t instr);
void mult(uint32_t *regFile, uint32_t instr);
int checkCaseTwo(uint32_t instr);
int checkCaseOne(uint32_t instr);
int checkInstruction(uint32_t instr);


struct state {
    uint32_t decoded;
    uint32_t fetched; 
};  


/* Selects the bits from a range when given a leftmost and a rightmost bit, 
and returns the selected bits as a 32-bit int */ 
uint32_t getBits(int leftmost, int rightmost, uint32_t num) {
    assert(leftmost >= rightmost);
    uint32_t mask = createMask(leftmost, rightmost);  
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
     
    // Allocating space for the registers 
    uint32_t *regFile = calloc(NUM_REG, sizeof(int));    
    
    // Stores the current state of the ARM machine  
    struct state *ARMState = (struct state*) malloc(sizeof(struct state)); 
    assert(ARMState != NULL);     
    
    ARMState->decoded = 0;
    ARMState->fetched = 0; 
    
    /* The pipeline begins its execution. The first instruction is fetched and 
    then decoded.*/
    
    ARMState->decoded = mainMem[0];       
   
    uint32_t condReg;    

    // infinite loop 
    for(;;) {
  
      // Checks if 
      if(ARMState->decoded == 0) {
            break;
        } 

    /* Checks the condition code of the instruction, which specifies whether
    the instruction will be executed or not */ 
    
    condReg = getBits(31, 28, ARMState->decoded); 
    if (checkCond(condReg, *(regFile + CPSR)) == 1){
        int instr = checkInstruction(switchEndian(ARMState->decoded));
        switch (instr) {
            case 0  : break; 
            case 1  : mult(regFile, instr); break;  
            case 2  : break; 
            default : assert(instr == 3); break;   
        }    
    } 
    
 
    ARMState->decoded = ARMState->fetched; 
    ARMState->fetched = mainMem[*regFile]; 
    *(regFile + PC) = *(regFile + PC) + 1;     

    }


    // PRINT STATE OF MEMORY/REGISTERS                                                   
    return 0;    
}

/*Determines the type of the instruction. Returns 0 if Data Processing, 
1 if multiply, 2 if Single Data Transfer, and 3 if Branch. */ 
int checkInstruction(uint32_t instr) { 
    
    // If bit 27 is set then the type is Branch    
    if (getBits(27, 27, instr) == 1) {
        return 3;
    // If bit 26 is set then the type is Single Data Transfer
    } else if (getBits(26, 26, instr) == 1) {
        return 2;
    } else {
        return checkCaseOne(instr);    
    }
           
} 

/* Determines whether the instruction is a Data Processing or Multiply
instruction by checking specific bits */ 
int checkCaseOne(uint32_t instr) {
    if (getBits(25, 25, instr) == 1) { 
        return 0;
    } else {
        if (getBits(4, 4, instr) == 0) { 
            return 0;
        } else {
            return checkCaseTwo(instr);    
        } 
    }
}


/* Determines whether the instruction is a Data Processing or Multiply
instruction by checking specific bits */ 
int checkCaseTwo(uint32_t instr) { 
    if (getBits(7, 7, instr) == 0) { 
        return 0;
    } else {
        return 1; 
    }
}

int checkCond(uint32_t cond, uint32_t CPSRreg) {
    /* This function is used to check the condition of the instruction.
    It calls helper functions checkZ and equalityNV to return a value
    of 1 if the condition is met, or 0 if the condition is not met. */
    switch(cond) {
        case 0  : return checkZ(CPSRreg); 
        case 1  : return !checkZ(CPSRreg);
        case 10 : return equalityNV(CPSRreg);
        case 11 : return !equalityNV(CPSRreg);
        case 12 : return (!checkZ(CPSRreg) && equalityNV(CPSRreg));
        case 13 : return (checkZ(CPSRreg) || !equalityNV(CPSRreg));
        default : assert(cond == 14); return 1;
    } 
}

int checkZ(uint32_t CPSRreg) {
    /* This function checks the 30th bit of the CPSR register (the Z
       bit) to see if it is set. Returns 1 if Z is set. Returns 0 if
       it is clear. */
    return getBits(30, 30, CPSRreg);
}

int equalityNV(uint32_t CPSRreg) {
    /* This function checks the 31st bit of the CPSR register (the N
       bit) and compares it with the 28th bit (the V bit). Returns 1 of
       N = C. Returns 0 if N != C */
    int N = getBits(31, 31, CPSRreg);
    int V = getBits(28, 28, CPSRreg);
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
    
    uint32_t mostSigBit = getBits(25, 25, offset);

    // check most significant bit to see if a sign extension is needed
    if(mostSigBit == 1){
        uint32_t endOfInstr = 31;
        uint32_t mostSigBitIndex = 25;
        uint32_t signExtension = createMask(endOfInstr, mostSigBitIndex);
        offset = signExtension | offset;
    }
    // most significant bit of 0 will not need a sign extension
    return offset;
}



void mult(uint32_t *regFile, uint32_t instr) {
 
    /* The bit value of the accumulate bit, which decides whether the
    instruction performs a multiply or multiply and accumulate */     
    uint32_t accBit = getBits(21, 21, instr); 

    /* The bit value of the set condition which decides wheter the CPSR
    flags are updated during execution */ 
    uint32_t setBit = getBits(20, 20, instr);  
    
    /* The result from performing the operation */
    uint32_t result;    
 
    if (accBit == 0) { 
        result = multInstr(regFile, instr);  
    } else {
        result = accMultInstr(regFile, instr); 
    } 

    if (setBit == 1) {

        // The pointer to the CPSR register 
        uint32_t *CPSRpntr = regFile + CPSR;   
       
        // The updated N and Z flag bit values        
        uint32_t N = getBits(31, 31, result);    
        uint32_t Z = (result == 0); 
       
        N <<= 31;     
        Z <<= 30;
    
        uint32_t NMask = createMask(30, 0);
        uint32_t ZMask = createMask(29, 0); 
        N |= NMask;
        Z |= ZMask;

        *CPSRpntr &= N;
        *CPSRpntr &= Z;     
    }  
}


/* Since the Accumulate bit is 0, then the multiply instruction is 
carried out, and performs Rd:=RmxRs, where Rn is ignored */ 
uint32_t multInstr(uint32_t *regFile, uint32_t instr) {
    uint32_t destReg = getBits(19, 16, instr);  
    uint32_t regS = getBits(11, 8, instr); 
    uint32_t regM = getBits(3, 0, instr); 
    regFile[destReg] = regFile[regM] * regFile[regS];   
    return regFile[destReg];  
}


/* Since the Accumulate bit is 1, then the accumulate-multiply instruction is 
carried out, and performs Rd:=RmxRs+Rn */ 
uint32_t accMultInstr(uint32_t *regFile, uint32_t instr) {
    uint32_t destReg = getBits(19, 16, instr); 
    uint32_t regN = getBits(15, 12, instr); 
    uint32_t regS = getBits(11, 8, instr); 
    uint32_t regM = getBits(3, 0, instr); 
    regFile[destReg] = (regFile[regM] * regFile[regS]) + regFile[regN];   
    return regFile[destReg];  
}

uint32_t createMask(uint32_t top, uint32_t bot) {
    /* createMask is a helper function that produces a mask (with correct
       offset) */
    uint32_t difference = top - bot;

    uint32_t mask = (1 << (difference + 1)) - 1;
    mask <<= bot;
    return mask;
}

void printState(uint32_t *regFile, uint32_t *mainMem, int memSize) { 
    int i; 
    for (i = 0; i < GEN_REG; i++) { 
        printf("%i  :          %i (%x)\n", i, regFile[i], regFile[i]);  
    }
    
    printf("PC  :          %i (%x)\n", regFile[15], regFile[15]);
    printf("CPSR:          %i (%x)\n", regFile[16], regFile[16]);
    printf("Non-zero memory:"); 
    
    for (i = 0; i < memSize; i++) { 
        printf("%x: %x\n", i, mainMem[i]); 
    }  
}





    
