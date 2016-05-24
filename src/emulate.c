#include "emulate.h"

int main(int argc, char **argv) {
    assert(argc == 2); 
    
    // Allocates space for the main memory and checks for error 
    mainMem = calloc(MAX_SIZE, sizeof(char)); 
    if (mainMem == NULL) { 
        perror("calloc");
        exit(EXIT_FAILURE);   
    }
    
    // Declaring the binary file  
    FILE *binFile; 
     
    // Defining the binary file with fopen 
    binFile = fopen(argv[1], "rb");
    
    if (binFile) {    
        fread(mainMem, sizeof(char), MAX_SIZE, binFile); 
    } else {
        perror("File not found!");
    }
   
    // Closes and disassociates the binary file  
    fclose(binFile); 
     
    // Allocates space for the registers and checks for error
    regFile = calloc(NUM_REG, sizeof(int));    
    if (regFile == NULL) { 
        perror("calloc");
        exit(EXIT_FAILURE);   
    }  
 
    // Stores the current state of the ARM machine and checks for error  
    struct state *ARMState = (struct state*) malloc(sizeof(struct state)); 
    if (ARMState == NULL) { 
        perror("malloc");
        exit(EXIT_FAILURE);   
    }     
   
    // Begins the pipeline process  
    simulatePipeline(ARMState); 
    
    // frees up dynamic memory
    free(mainMem);
    free(regFile);
    free(ARMState);
 
    return EXIT_SUCCESS;    
}

/* Simulates the pipeline process which executes, decodes and fetches 
instructions from the main memory */  
void simulatePipeline(struct state *ARMState) { 
   
    ARMState->decoded = 0;
    ARMState->fetched = 0; 
    
    /* The pipeline begins its execution. The first instruction is fetched and 
    then decoded.*/
    
    ARMState->fetched = getInteger(0);       
    regFile[PC] += sizeof(int); 
 
    ARMState->decoded = ARMState->fetched; 
    ARMState->fetched = getInteger(regFile[PC]);    
    regFile[PC] += sizeof(int);  
    uint32_t condReg;    
    
    // Start of infinite loop 
    for(;;) {
        // If instruction is 0, halt the program (break out of for loop) 
        if (ARMState->decoded == 0) {
            break;
        } 

        /* Checks the condition code of the instruction, which specifies whether
           the instruction will be executed or not */ 
        condReg = getBits(31, 28, ARMState->decoded);  
        if (checkCond(condReg) == 1){
            Instruction type = checkInstruction(ARMState->decoded);
            switch(type) {
                // DATA PROCESSING
                case DP  :
                    dataProcessInstr(ARMState->decoded);  
                    break;
                // MULTIPLY 
                case MUL  : 
                    mult(ARMState->decoded); 
                    break;  
                // SINGLE DATA TRANSFER
                case SDT  : 
                    singleDataTransfer(ARMState->decoded); 
                    break;  
                // BRANCH    
                default :
                    assert(type == BR); 
                    int offset = branch(ARMState->decoded); 
                    regFile[PC] += offset;   
                    
                    // Fetched is the instruction at PC + offset  
                    ARMState->fetched = getInteger(regFile[PC]);    
                    regFile[PC] += sizeof(int);
                    break;   
            }    
        } 
        // fetched instruction becomes decoded, and new instruction is fetched
        ARMState->decoded = ARMState->fetched;
        ARMState->fetched = getInteger(regFile[PC]);  
        // PC = PC + 4;  
        regFile[PC] += sizeof(int);   
    }

    // prints the state of processor upon termination
    printState(); 
}  

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

/* Extracts the first 4 bytes from the address and merges them into a 32 bit
instruction in Big Endian */ 
uint32_t getInteger(uint32_t firstByteAddr) { 
    uint32_t firstByte = mainMem[firstByteAddr];
    firstByte <<= (CHAR_BIT * 3);
    uint32_t secondByte = mainMem[firstByteAddr + sizeof(uint8_t)]; 
    secondByte <<= (CHAR_BIT * 2);
    uint32_t thirdByte = mainMem[firstByteAddr + sizeof(uint8_t) * 2]; 
    thirdByte <<= CHAR_BIT;
    uint32_t fourthByte = mainMem[firstByteAddr + sizeof(uint8_t) * 3];
    return switchEndian(firstByte | secondByte | thirdByte | fourthByte);     
}

/*Determines the type of the instruction. Returns 0 if Data Processing, 
1 if multiply, 2 if Single Data Transfer, and 3 if Branch. */ 
Instruction checkInstruction(uint32_t instr) {  
    // If bit 27 is set then the type is Branch    
    if (getBits(27, 27, instr) == 1) {
        return BR;
    // If bit 26 is set then the type is Single Data Transfer
    } else if (getBits(26, 26, instr) == 1) {
        return SDT;
    } else {
        return checkCaseOne(instr);    
    }
           
} 

/* This function is used to check the condition of the instruction.
   It calls helper functions checkZ and equalityNV to return a value
   of 1 if the condition is met, or 0 if the condition is not met. */
int checkCond(uint32_t cond) {
    switch(cond) {
        case 0  : 
            return checkZ(); 
        case 1  : 
            return !checkZ();
        case 10 : 
            return equalityNV();
        case 11 : 
            return !equalityNV();
        case 12 : 
            return (!checkZ() && equalityNV());
        case 13 : 
            return (checkZ() || !equalityNV());
        default : 
            assert(cond == 14); 
            return 1;
    } 
}

/* Determines whether the instruction is a Data Processing or Multiply
instruction by checking specific bits */ 
Instruction checkCaseOne(uint32_t instr) {
    if (getBits(25, 25, instr) == 1) { 
        return DP;
    } else {
        if (getBits(4, 4, instr) == 0) { 
            return DP;
        } else {
            return checkCaseTwo(instr);    
        } 
    }
}

/* Determines whether the instruction is a Data Processing or Multiply
instruction by checking specific bits */ 
Instruction checkCaseTwo(uint32_t instr) { 
    if (getBits(7, 7, instr) == 0) { 
        return DP;
    } else {
        return MUL; 
    }
}

/* This function checks the 30th bit of the CPSR register (the Z
   bit) to see if it is set. Returns 1 if Z is set. Returns 0 if
   it is clear. */ 
int checkZ(void) { 
   return getBits(30, 30, *(regFile + CPSR));
}

/* This function checks the 31st bit of the CPSR register (the N
   bit) and compares it with the 28th bit (the V bit). Returns 1 of
   N = C. Returns 0 if N != C */
int equalityNV(void) {
    int N = getBits(31, 31, *(regFile + CPSR));
    int V = getBits(28, 28, *(regFile + CPSR));
    return (N & V);
}
	    
/* branch is a helper function that contains an 24 bit 2s complement
   offset that will be sign extended and added to "PC" */
int branch(uint32_t instr) {
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

// Multiplies the contents of two registers with the option to accumulate
void mult(uint32_t instr) {
    /* The bit value of the accumulate bit, which decides whether the
    instruction performs a multiply or multiply and accumulate */     
    uint32_t accBit = getBits(21, 21, instr); 

    /* The bit value of the set condition which decides wheter the CPSR
    flags are updated during execution */ 
    uint32_t setBit = getBits(20, 20, instr);  
    
    /* The result from performing the operation */
    uint32_t result;    
 
    if (accBit == 0) { 
        result = multInstr(instr);  
    } else {
        result = accMultInstr(instr); 
    } 

    if (setBit == 1) {
        // The updated N and Z flag bit values        
        uint32_t N = getBits(31, 31, result);    
        uint32_t Z = (result == 0); 
        
        setCPSRBit(31, N);       
        setCPSRBit(30, Z);  
    }  
}


/* Since the Accumulate bit is 0, then the multiply instruction is 
carried out, and performs Rd:=RmxRs, where Rn is ignored */ 
uint32_t multInstr(uint32_t instr) {
    uint32_t destReg = getBits(19, 16, instr);  
    uint32_t regS = getBits(11, 8, instr); 
    uint32_t regM = getBits(3, 0, instr); 
    regFile[destReg] = regFile[regM] * regFile[regS];   
    return regFile[destReg];  
}


/* Since the Accumulate bit is 1, then the accumulate-multiply instruction is 
carried out, and performs Rd:=RmxRs+Rn */ 
uint32_t accMultInstr(uint32_t instr) {
    uint32_t destReg = getBits(19, 16, instr); 
    uint32_t regN = getBits(15, 12, instr); 
    uint32_t regS = getBits(11, 8, instr); 
    uint32_t regM = getBits(3, 0, instr); 
    regFile[destReg] = (regFile[regM] * regFile[regS]) + regFile[regN];   
    return regFile[destReg];  
}

/* createMask is a helper function that produces a mask (with correct
   offset) */
uint32_t createMask(uint32_t top, uint32_t bot) {
    uint32_t difference = top - bot;

    uint32_t mask = (1 << (difference + 1)) - 1;
    mask <<= bot;
    return mask;
}

// Prints the state of the processor
void printState(void) { 
    int i; 
    for (i = 0; i < GEN_REG; i++) { 
        printf("%i\t:  %d (0x%08x)\n", i, regFile[i], regFile[i]);  
    }
    
    printf("PC  :          %d (0x%08x)\n", regFile[PC], regFile[PC]);
    printf("CPSR:          %d (0x%08x)\n", regFile[CPSR], regFile[CPSR]);
    printf("Non-zero memory:\n");
    for (i = 0; i < MAX_SIZE; i += sizeof(int)) {
        if (getInteger(i) != 0) {
            printf("0x%08x: 0x%08x\n", i, switchEndian(getInteger(i)));
        }
    }
}

/* Moves each bit to the left by the specified amount, extending with 0s */
uint32_t logicalLeftShift(uint32_t amount, uint32_t value) {
    return (value << amount);
}

/* Moves each bit to the right by the specified amount, extending with 0s */
uint32_t logicalRightShift(uint32_t amount, uint32_t value) {
    return (value >> amount);
}

/* Makes an arithmetic shift, while preserving the sign bit */
uint32_t arithmeticRightShift(uint32_t amount, uint32_t value) {
    uint32_t leftMostBit = getBits(31, 31, value);
    // Logical right shift
    uint32_t shiftedValue = (value >> amount);
    if (leftMostBit != 0) {
        // Need to sign extend it
        uint32_t mask = createMask(31, 31 - amount + 1);
        shiftedValue |= mask;
    }
    return shiftedValue;
}

/* Rotates cyclically the value the amount number of bits */
uint32_t rotateRight(uint32_t amount, uint32_t value) {
    // Getting the bits to rotate
    uint32_t bitsToRotate = getBits(amount - 1, 0, value);
    // Moving the bits to the begginning
    bitsToRotate <<= (32 - amount);
    // Logical right shift of value
    uint32_t result = value >> amount;
    // Doing a bit-wise or to insert the bits to rotate
    return (result | bitsToRotate);
}

/* Given the operand determines the amount to be shifted, taking into account
    whether it is a constant amount or specified by a register */
uint32_t getShiftAmount(uint32_t operand) {
    uint32_t shiftKind = getBits(4, 4, operand);
    uint32_t shiftAmount;
    if (shiftKind == 0) {
        // Initialise shiftAmount as the constant
        shiftAmount = getBits(11, 7, operand);
    } else {
        // Change shiftAmount to the last byte in the shift Reg
        uint32_t shiftReg = getBits(11, 8, operand);
        uint32_t shiftRegValue = regFile[shiftReg];
        shiftAmount = getBits(7, 0, shiftRegValue);
    }
    return shiftAmount;
}

/* Given an operand, outputs the value after the shifted register operation
    is applied to the value in Rm (bits 0 to 3) */
uint32_t evaluateShiftedReg(uint32_t operand) {
    // Gets the value on which the shift operations will be applied (Rm)
    uint32_t valueReg = getBits(3, 0, operand);
    uint32_t shiftValue = regFile[valueReg];
    
    // Gets the amount to be shifted
    uint32_t shiftAmount = getShiftAmount(operand);

    // Get the shift type
    uint32_t shiftType = getBits(6, 5, operand);
    
    switch (shiftType) {
        case 0 : 
            return logicalLeftShift(shiftAmount, shiftValue);
        case 1 : 
            return logicalRightShift(shiftAmount, shiftValue);
        case 2 : 
            return arithmeticRightShift(shiftAmount, shiftValue);
        default: 
            assert(shiftType == 3);
            return rotateRight(shiftAmount, shiftValue);
    }
}

/* Helper function for single data transfer that stores the data from the source
    register to the main memory */
void storeData(uint32_t source, uint32_t index) {
    if (index < MAX_SIZE) {
        uint32_t data = switchEndian(regFile[source]);
        mainMem[index] = getBits(31, 24, data);
        mainMem[index + 1] = getBits(23, 16, data);
        mainMem[index + 2] = getBits(15, 8, data);
        mainMem[index + 3] = getBits(7, 0, data);
    } else {
        printf("Error: Out of bounds memory access at address 0x%08x\n", index);
    }
}

/* Helper function for single data transfer that loads the data from the main
    memory into the dest register */
void loadData(uint32_t dest, uint32_t index) {
    if (index < MAX_SIZE) {
        uint32_t data = getInteger(index);
        regFile[dest] = data;
    } else {
        printf("Error: Out of bounds memory access at address 0x%08x\n", index);
    }
}

/* This function is used to do single data transfers. The instuction determines
    whether it is a load or a store and the offset for the base register */
void singleDataTransfer(uint32_t instr) {
    // Initialise the offset
    uint32_t offset = getBits(11, 0, instr);
    // Get the Immediate offset(I) bit
    uint32_t immBit = getBits(25, 25, instr);
    // If the Immediate offset flag is set, get the shited register value
    if (immBit != 0) {
        offset = evaluateShiftedReg(offset);
    }
    
    // Get the up bit
    uint32_t upBit = getBits(23, 23, instr);
    
    //Get load/store bit
    uint32_t loadStoreBit = getBits(20, 20, instr);
    
    // Get the base register (Rn)
    uint32_t baseReg = getBits(19, 16, instr);

    // Get the destination register
    uint32_t destReg = getBits(15, 12, instr);
   
    // Set the baseReg index
    int32_t index = regFile[baseReg];

    // Get the Pre/Post indexing bit
    uint32_t pIndexingBit = getBits(24, 24, instr);
    if (pIndexingBit != 0) {
    // Pre indexing
        if (upBit != 0) {
            // Offset is added to index
            index += offset;
        } else {
            // Offset is subtracted from index
            index -= offset;
        }
        if (loadStoreBit == 0) {
            storeData(destReg, index);
        } else {
            loadData(destReg, index);
        }
    } else {
    // Post indexing
        // In a post-indexing have to check that Rm and Rn are not the same
        if (immBit != 0) {
            // Get Rm (offset register)
            uint32_t offsetReg = getBits(3, 0, instr);
            assert (offsetReg != baseReg);
        }
        if (loadStoreBit == 0) {
            storeData(destReg, index);
        } else {
            loadData(destReg, index);
        }

        // Check up bit
        if (upBit != 0) {
            // Offset is added to base reg
            regFile[baseReg] += offset;
        } else {
            // Offset is subtracted from base reg
            regFile[baseReg] -= offset;
        }
    }
}

/* This function is called by the data processing instruction if the
   immediate operand bit (bit 25) of the instruction is set. The function
   takes an 8 bit immediate value (bits 0 - 7) and rotates it by twice 
   the value of the rotate bits (bits 8-11). */
uint32_t evaluateImmediateValue(uint32_t operand) {
    // the rotate value must be a multiple of 2
    uint32_t rotateValue = getBits(11, 8, operand);

    /* the immediate value does not need to be zero-extended as it is declared
       as a uint32_t */
    uint32_t immValue = getBits(7, 0, operand);
    
    // any rotation amount is twice the value in the 4 bit rotate field
    rotateValue <<= 1;

    // the immediate value is rotated right by rotateValue
    if (rotateValue >= 1) {
        immValue = rotateRight(rotateValue, immValue);
    }

    return immValue;
}

/* This function is called by main. If the immediate operand is set
   then operand2 is an immediate value. If immediate operand is clear then
   operand2 is a register. Also sets flags depending on the S bit(bit 20)
   of the instruction. */
void dataProcessInstr(uint32_t instr) { 
    // get the 25th bit of the instruction (immediate operand bit)
    uint32_t immOp = getBits(25, 25, instr);
    
    // get the operand2 of the instruction
    uint32_t op2Bits = getBits(11, 0, instr);
    uint32_t op2;

    if (immOp != 0) {
    // if the immediate operand bit is 1. op2 is an immediate value
        op2 = evaluateImmediateValue(op2Bits);
    } else {
    // else, the immediate operand bit is 0. op2 is a register
        op2 = evaluateShiftedReg(op2Bits);
    }
  
    // get the opcode from the instruction, and regN from instruction
    uint32_t opcode = getBits(24, 21, instr);
    uint32_t regN = getBits(19, 16, instr);

    // result from applying opcode
    uint32_t result = executeOpcode(opcode, regN, op2);
    
    if (opcode < 8 || opcode > 10) {
        // the register to put result into
        uint32_t destReg = getBits(15, 12, instr);
        regFile[destReg] = result;
    }
    
    // get the 20th bit of the instruction (set condition bit)
    uint32_t set = getBits(20, 20, instr);

    if (set != 0) {
        // V bit of CPSR is not altered
    
        uint32_t carry;
        /* C bit of CPSR is altered according to the type of shift
           (left or right) */
        switch(opcode) {
            case 0  :
            case 1  :
            case 8  :
            case 9  :
            case 12 :
            case 13 : 
                carry = getCarryFromShifter(op2Bits); 
                break;
            case 2  :
            case 3  :
            case 4  :
            case 10 : 
                carry = getCarryFromALU(instr, op2);
                break;
            default : 
                perror("The opcode entered was not valid, carry\
                        not produced.");
        }
        
        // set C flag (bit 29)
        setCPSRBit(29, carry);    
    
        if (result == 0) {
            // if result == 0, set Z flag (bit 30)
            setCPSRBit(30, 1);
        } else {
            // if result != 0, clear Z flag (bit 30)
            setCPSRBit(30, 0);
        }

        // set N flag (bit 31)
        int32_t bit31 = getBits(31, 31, result);
        setCPSRBit(31, bit31);
    }
}

// The carry is extracted from ALU operations
uint32_t getCarryFromALU(uint32_t instr, uint32_t op2) {
    uint32_t opcode = getBits(24, 21, instr);
    assert(opcode == 2 || opcode == 3 || opcode == 4 || opcode == 10);

    uint32_t regN = getBits(19, 16, instr);
    uint32_t op1 = regFile[regN];

    if(opcode == 4) {
        // if add
        if(isOverflow(op1, op2) == 1) {
            // unsigned addition overflow occured
            return 1;
        }
    } else if(opcode == 2 || opcode == 10) {
        // if sub or cmp
        if(op1 >= op2) {
            // no borrow was produced
            return 1;
        }
    } else if(opcode == 3) {
        if(op2 >= op1) {
            // rsb is well formed, no borrow produced
            return 1;
        }
    }
    return 0;        
}

// Checks if an overflow will occur if int1 and int2 are added together.
uint32_t isOverflow(uint32_t int1, uint32_t int2) {
    if(int1 > (INT_MAX - int2)){
        return 1;
    }
    return 0;
} 

// Gets carry from shifters when op2 is a register
uint32_t getCarryFromShifter(uint32_t op2Bits) {
    uint32_t shiftType = getBits(6, 5, op2Bits);
    uint32_t shiftAmount = getShiftAmount(op2Bits);    
    uint32_t carry;
    uint32_t regM = getBits(3, 0, op2Bits);

    if(shiftAmount == 0) {
        // there was no shift, hence no carry
        return 0;
    }

    if(shiftType == 0) {
        // shiftType is logical shift left, so add one to shiftAmount
        shiftAmount -= 1;
        carry = getBits(31 - shiftAmount, 31 - shiftAmount, regFile[regM]);
    } else {
        // shiftType is a right shift, so subtract one from shiftAmount
        shiftAmount -= 1;
        carry = getBits(shiftAmount, shiftAmount, regFile[regM]);
    }

    return carry;
}

// Decodes opcode and applies it to Rn and op2
uint32_t executeOpcode(uint32_t opcode, uint32_t regN, uint32_t op2) {
    assert(opcode <= 13);
    
    switch(opcode) {
        case 0  : 
        case 8  : 
            return regFile[regN] & op2;
        case 1  :
        case 9  : 
            return regFile[regN] ^ op2;
        case 2  : 
        case 10 : 
            return regFile[regN] - op2;
        case 3  : 
            return op2 - regFile[regN];
        case 4  : 
            return regFile[regN] + op2;
        case 12 : 
            return regFile[regN] | op2;
        default : 
            assert(opcode == 13);
            return op2;
    } 
}

/* This function is called to set the specified bit of CPSR register
   to the given value */
void setCPSRBit(uint32_t bit, uint32_t value) {
    assert(bit <= 31 && bit >= 28);
    assert(value == 0 || value == 1);
   
    // create a pointer to the CPSR register.
    uint32_t *CPSRreg = regFile + CPSR;
 
    int32_t mask = createMask(bit, bit);
    
    if (value == 0) {
        // invert the mask
        mask = ~mask;
        // clear the bit
        *CPSRreg &= mask;   
    } else {
        // set the bit
        *CPSRreg |= mask;
    } 
}       
