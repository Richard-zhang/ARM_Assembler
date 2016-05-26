#include "emulate.h"

int main(int argc, char **argv) {
    assert(argc == 2); 

    struct state *ARMState = NULL;
    // Allocates space for the main memory and checks for error 
    mainMem = calloc(MAX_SIZE, sizeof(char)); 
    if (mainMem == NULL) { 
    	freeAllMemory(ARMState);
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
    	freeAllMemory(ARMState);
        perror("calloc");
        exit(EXIT_FAILURE);   
    }  
 
    // Stores the current state of the ARM machine and checks for error  
    ARMState = (struct state*) malloc(sizeof(struct state)); 
    if (ARMState == NULL) { 
    	freeAllMemory(ARMState);
        perror("malloc");
        exit(EXIT_FAILURE);   
    }     
   
    // Begins the pipeline process  
    simulatePipeline(ARMState); 
    
    // frees up dynamic memory
    freeAllMemory(ARMState);
 
    return EXIT_SUCCESS;    
}

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

Instruction checkCaseTwo(uint32_t instr) { 
    if (getBits(7, 7, instr) == 0) { 
        return DP;
    } else {
        return MUL; 
    }
}

void printState(void) { 
    int i;
    printf("Registers:\n");  
    for (i = 0; i < GEN_REG; i++) { 
        printf("$%-3i: %10d (0x%08x)\n", i, regFile[i], regFile[i]);  
    }
    
    printf("PC  : %10d (0x%08x)\n", regFile[PC], regFile[PC]);
    printf("CPSR: %10d (0x%08x)\n", regFile[CPSR], regFile[CPSR]);
    printf("Non-zero memory:\n");
    for (i = 0; i < MAX_SIZE; i += sizeof(int)) {
        if (getInteger(i) != 0) {
            printf("0x%08x: 0x%08x\n", i, switchEndian(getInteger(i)));
        }
    }
}
 
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
        condReg = getBits(MS_BIT, LSCOND_BIT, ARMState->decoded);  
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

void freeAllMemory(struct state *ARMState) {
    if (mainMem != NULL) {
        free(mainMem);
    }
    
    if (regFile != NULL) {
        free(regFile);
    }

    if (ARMState != NULL) {
        free(ARMState);
    }
}

void dataProcessInstr(uint32_t instr) { 
    // get the 25th bit of the instruction (immediate operand bit)
    uint32_t immOp = getBits(I_BIT, I_BIT, instr);
    
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
            setCPSRBit(COND_Z_BIT, 1);
        } else {
            // if result != 0, clear Z flag (bit 30)
            setCPSRBit(COND_Z_BIT, 0);
        }

        // set N flag (bit 31)
        int32_t bit31 = getBits(MS_BIT, MS_BIT, result);
        setCPSRBit(MS_BIT, bit31);
    }
}

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

uint32_t getCarryFromShifter(uint32_t op2Bits) {
    uint32_t shiftType = getBits(6, 5, op2Bits);
    uint32_t shiftAmount = getShiftAmount(op2Bits);    
    uint32_t carry;
    uint32_t regM = getBits(3, 0, op2Bits);

    if(shiftAmount == 0) {
        // there was no shift, hence no carry
        return 0;
    }
    
    shiftAmount -= 1;

    if(shiftType == 0) {
        carry = getBits(MS_BIT - shiftAmount, MS_BIT - shiftAmount, 
                        regFile[regM]);
    } else {
        carry = getBits(shiftAmount, shiftAmount, regFile[regM]);
    }
    return carry;
}

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

uint32_t isOverflow(uint32_t int1, uint32_t int2) {
    if(int1 > (INT_MAX - int2)){
        return 1;
    }
    return 0;
} 

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
        uint32_t N = getBits(MS_BIT, MS_BIT, result);    
        uint32_t Z = (result == 0); 
        setCPSRBit(MS_BIT, N);       
        setCPSRBit(30, Z);  
    }  
}
 
uint32_t multInstr(uint32_t instr) {
    uint32_t destReg = getBits(19, 16, instr);  
    uint32_t regS = getBits(11, 8, instr); 
    uint32_t regM = getBits(3, 0, instr); 
    regFile[destReg] = regFile[regM] * regFile[regS];   
    return regFile[destReg];  
}

uint32_t accMultInstr(uint32_t instr) {
    uint32_t destReg = getBits(19, 16, instr); 
    uint32_t regN = getBits(15, 12, instr); 
    uint32_t regS = getBits(11, 8, instr); 
    uint32_t regM = getBits(3, 0, instr); 
    regFile[destReg] = (regFile[regM] * regFile[regS]) + regFile[regN];   
    return regFile[destReg];  
}

void singleDataTransfer(uint32_t instr) {
    // Initialise the offset
    uint32_t offset = getBits(11, 0, instr);
    // Get the Immediate offset(I) bit
    uint32_t immBit = getBits(I_BIT, I_BIT, instr);
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

uint32_t getShiftAmount(uint32_t operand) {
    uint32_t shiftKind 
                = getBits(SHFT_REG_KIND_BIT, SHFT_REG_KIND_BIT, operand);
    uint32_t shiftAmount;
    if (shiftKind == 0) {
        // Initialise shiftAmount as the constant
        // SHFT_REG_CONST_MS_BIT = 11; SHFT_REG_CONST_LS_BIT = 7
        shiftAmount = getBits(SHFT_REG_CONST_MS_BIT, 
                                        SHFT_REG_CONST_LS_BIT, operand);
    } else {
        // Change shiftAmount to the last byte in the shift Reg
        uint32_t shiftReg = getBits(SHFT_REG_CONST_MS_BIT, 8, operand);
        uint32_t shiftRegValue = regFile[shiftReg];
        shiftAmount = getBits(7, LS_BIT, shiftRegValue);
    }
    return shiftAmount;
}

void storeData(uint32_t source, uint32_t index) {
    if (index < MAX_SIZE) {
        uint32_t data = switchEndian(regFile[source]);
        mainMem[index] = getBits(MS_BIT, FST_B_LS_BIT, data);
        mainMem[index + 1] = getBits(SND_B_MS_BIT, SND_B_LS_BIT, data);
        mainMem[index + 2] = getBits(THRD_B_MS_BIT, THRD_B_LS_BIT, data);
        mainMem[index + 3] = getBits(FRTH_B_MS_BIT, LS_BIT, data);
    } else {
        printf("Error: Out of bounds memory access at address 0x%08x\n", index);
    }
}

void loadData(uint32_t dest, uint32_t index) {
    if (index < MAX_SIZE) {
        uint32_t data = getInteger(index);
        regFile[dest] = data;
    } else {
        printf("Error: Out of bounds memory access at address 0x%08x\n", index);
    }
}

uint32_t branch(uint32_t instr) {
    // extracts offset from the instruction
    // BR_MS_OFFSET_BIT = 23
    int32_t offset = getBits(BR_MS_OFFSET_BIT, 0, instr);
    
    /* offset is shifted left 2 bits to account for the PC being 8 bytes
       ahead of the instruction that is being executed */  
    int shift = 2;
    offset <<= shift;

    // BR_LS_OPCODE_BIT = 25
    uint32_t mostSigBit 
                    = getBits(BR_LS_OPCODE_BIT, BR_LS_OPCODE_BIT, offset);

    // check most significant bit to see if a sign extension is needed
    if(mostSigBit == 1){
        uint32_t endOfInstr = MS_BIT;
        uint32_t mostSigBitIndex = BR_MS_OFFSET_BIT + shift;
        uint32_t signExtension = createMask(endOfInstr, mostSigBitIndex);
        offset = signExtension | offset;
    }
    // most significant bit of 0 will not need a sign extension
    return offset;
}

uint32_t checkCond(uint32_t cond) {
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

uint32_t checkZ(void) { 
   return getBits(COND_Z_BIT, COND_Z_BIT, *(regFile + CPSR));
}

uint32_t equalityNV(void) {
    int N = getBits(MS_BIT, MS_BIT, *(regFile + CPSR));
    int V = getBits(LSCOND_BIT, LSCOND_BIT, *(regFile + CPSR));
    return (N & V);
}

uint32_t logicalLeftShift(uint32_t amount, uint32_t value) {
    return (value << amount);
}

uint32_t logicalRightShift(uint32_t amount, uint32_t value) {
    return (value >> amount);
}

uint32_t arithmeticRightShift(uint32_t amount, uint32_t value) {
    uint32_t leftMostBit = getBits(MS_BIT, MS_BIT, value);
    // Logical right shift
    uint32_t shiftedValue = (value >> amount);
    if (leftMostBit != 0) {
        // Need to sign extend it
        uint32_t mask = createMask(MS_BIT, MS_BIT - amount + 1);
        shiftedValue |= mask;
    }
    return shiftedValue;
}

uint32_t rotateRight(uint32_t amount, uint32_t value) {
    // Getting the bits to rotate
    uint32_t bitsToRotate = getBits(amount - 1, LS_BIT, value);
    // Moving the bits to the begginning
    bitsToRotate <<= (sizeof(int) * CHAR_BIT - amount);
    // Logical right shift of value
    uint32_t result = value >> amount;
    // Doing a bit-wise or to insert the bits to rotate
    return (result | bitsToRotate);
}

void setCPSRBit(uint32_t bit, uint32_t value) {
    assert(bit <= MS_BIT && bit >= LSCOND_BIT);
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

uint32_t switchEndian(uint32_t num) {  
    uint32_t a = (num & 0x000000ff)  << (CHAR_BIT * 3); 
    uint32_t b = (num & 0x0000ff00)  << CHAR_BIT;
    uint32_t c = (num & 0x00ff0000)  >> CHAR_BIT;
    uint32_t d = (num & 0xff000000)  >> (CHAR_BIT * 3);
    return a | b | c | d;
}  

uint32_t getBits(uint32_t leftmost, uint32_t rightmost, uint32_t num) {
    assert(leftmost >= rightmost);
    uint32_t mask = createMask(leftmost, rightmost);  
    num &= mask;
    num >>= rightmost;
    return num;   
}

uint32_t createMask(uint32_t top, uint32_t bot) {
    uint32_t difference = top - bot;

    uint32_t mask = (1 << (difference + 1)) - 1;
    mask <<= bot;
    return mask;
}
