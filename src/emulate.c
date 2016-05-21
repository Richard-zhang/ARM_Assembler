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
uint32_t logicalLeftShift(uint32_t amount, uint32_t value);
uint32_t logicalRightShift(uint32_t amount, uint32_t value);
uint32_t arithmeticRightShift(uint32_t amount, uint32_t value);
uint32_t rotateRight(uint32_t amount, uint32_t value);
uint32_t getShiftAmount(uint32_t *regFile, uint32_t operand);
uint32_t evaluateShiftedReg(uint32_t *regFile, uint32_t operand);
uint32_t evaluateImmediateValue(uint32_t operand);
void setCPSRBit(uint32_t *CPSRreg, uint32_t bit, uint32_t value);
uint32_t applyOpcode(uint32_t *regFile, uint32_t opcode, uint32_t regN,
                     uint32_t op2);
uint32_t getCarryFromShifter(uint32_t *regFile, uint32_t op2);
uint32_t isOverflow(uint32_t int1, uint32_t int2);
uint32_t getCarryFromALU(uint32_t *regFile, uint32_t instr, uint32_t op2,
                         uint32_t *CPSRreg);
void dataProcessInstr(uint32_t *regFile, uint32_t instr);

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
            case 1  : mult(regFile, switchEndian(ARMState->decoded)); break;  
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

uint32_t logicalLeftShift(uint32_t amount, uint32_t value) {
    return (value << amount);
}

uint32_t logicalRightShift(uint32_t amount, uint32_t value) {
    return (value >> amount);
}

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

uint32_t getShiftAmount(uint32_t *regFile, uint32_t operand) {
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

uint32_t evaluateShiftedReg(uint32_t *regFile, uint32_t operand) {
    // Gets the value on which the shift operations will be applied (Rm)
    uint32_t valueReg = getBits(3, 0, operand);
    uint32_t shiftValue = regFile[valueReg];
    
    // Gets the amount to be shifted
    uint32_t shiftAmount = getShiftAmount(regFile, operand);

    // Get the shift type
    uint32_t shiftType = getBits(6, 5, operand);
    
    switch (shiftType) {
        case 0 : return logicalLeftShift(shiftAmount, shiftValue);
        case 1 : return logicalRightShift(shiftAmount, shiftValue);
        case 2 : return arithmeticRightShift(shiftAmount, shiftValue);
        default: assert(shiftType == 3);
                 return rotateRight(shiftAmount, shiftValue);
    }
}

uint32_t evaluateImmediateValue(uint32_t operand) {
    /* This function is called by the data processing instruction if the
       immediate operand bit (bit 25) of the instruction is set. The function
       takes an 8 bit immediate value (bits 0 - 7) and rotates it by twice 
       the value of the rotate bits (bits 8-11). */

    // the rotate value must be a multiple of 2
    uint32_t rotateValue = getBits(11, 8, operand);

    /* the immediate value does not need to be zero-extended as it is declared
       as a uint32_t */
    uint32_t immValue = getBits(0, 7, operand);
    
    // any rotation amount is twice the value in the 4 bit rotate field
    rotateValue <<= 2;

    // the immediate value is rotated right by rotateValue
    immValue >>= rotateValue;

    return immValue;
}

void dataProcessInstr(uint32_t *regFile, uint32_t instr) {
    /* This function is called by the main. If the immediate operand is set
       then operand2 is an immediate value. If immediate operand is clear then
       operand2 is a register. Also sets flags depending on the S bit(bit 20)
       of the instruction. */

    // create a pointer to the CPSR register.
    uint32_t *CPSRreg = regFile + CPSR;
    
    // get the 25th bit of the instruction (immediate operand bit)
    uint32_t immOp = getBits(25, 25, instr);

    // get the 20th bit of the instruction (set condition bit)
    uint32_t set = getBits(20, 20, instr);

    // get the operand2 of the instruction
    uint32_t op2 = getBits(11, 0, instr);

    if(immOp != 0) {
    // if the immediate operand bit is 1. op2 is an immediate value
        op2 = evaluateImmediateValue(op2);
    } else {
    // else, the immediate operand bit is 0. op2 is a register
        op2 = evaluateShiftedReg(regFile, op2);
    }
  
    // get the opcode from the instruction, and regN from instruction
    uint32_t opcode = getBits(24, 21, instr);
    uint32_t regN = getBits(19, 16, instr);

    // result from applying opcode
    uint32_t result = applyOpcode(regFile, opcode, regN, op2);
    
    // the register to put result into
    uint32_t destReg = getBits(15, 12, instr);
   
    if(opcode < 8 || opcode > 10) {
        regFile[destReg] = result;
    }
    
    if(set != 0) {
        // V bit of CPSR is not altered
    
        uint32_t carry;
        /* C bit of CPSR is altered according to the type of shift
           (left or right) */
        if(immOp == 0) {
            // op2 was a register
            switch(opcode) {
                case 0  :
                case 1  :
                case 8  :
                case 9  :
                case 12 :
                case 13 : carry = getCarryFromShifter(regFile, op2); break;
                case 2  :
                case 3  :
                case 4  :
                case 10 : carry = getCarryFromALU(regFile, instr, op2, CPSRreg);
                          break;
                default : perror("The opcode entered was not valid, carry\
                              not produced.");
            }
        } else {
            // op2 was an immediate value
            uint32_t shiftAmount = getBits(11, 8, instr);
            uint32_t imm = getBits(7, 0, instr);
            shiftAmount <<= 2;
        
            // set correct shiftAmount for right rotations
            shiftAmount -= 1;
            carry = getBits(shiftAmount, shiftAmount, imm);        
        }
    
        // set C flag (bit 29)
        setCPSRBit(CPSRreg, 29, carry);    

        if(result == 0) {
            // set Z flag (bit 30)
            setCPSRBit(CPSRreg, 30, 1);
        }

        // set N flag (bit 31)
        int32_t bit31 = getBits(31, 31, result);
        setCPSRBit(CPSRreg, 31, bit31);
    }
}

uint32_t getCarryFromALU(uint32_t *regFile, uint32_t instr, uint32_t op2,
                         uint32_t *CPSRreg) 
{
    uint32_t opcode = getBits(24, 21, instr);
    assert(opcode == 2 || opcode == 3 || opcode == 4 || opcode == 10);

    uint32_t regN = getBits(19, 16, instr);
    uint32_t op1 = regFile[regN];

    if(opcode == 4) {
        // if add
        if(isOverflow(op1, op2) == 1) {
            // overflow occured
            return 1;
        }
    } else if(opcode == 2 || opcode == 10) {
        // if sub or cmp
        if(op1 > op2) {
            return 1;
        }
    } else if(opcode == 3) {
        // if rsb
        if(op2 > op1) {
            return 1;
        }
    }
    return 0;        
}

uint32_t isOverflow(uint32_t int1, uint32_t int2) {
    if((int2 > 0) && (int1 > (sizeof(uint32_t) - int2))){
        return 1;
    }

    return 0;
} 

uint32_t getCarryFromShifter(uint32_t *regFile, uint32_t op2) {
    uint32_t shiftType = getBits(6, 5, op2);
    uint32_t shiftAmount = getShiftAmount(regFile, op2);    
    uint32_t carry;
    uint32_t regM = getBits(3, 0, op2);

    if(shiftAmount == 0) {
        // there was not shift, hence no carry
        return 0;
    }

    if(shiftType == 0) {
        // shiftType is logical shift left, so add one to shiftAmount
        shiftAmount += 1;
        carry = getBits(shiftAmount, shiftAmount, regFile[regM]);
    } else {
        // shiftType is a right shift, so subtract one from shiftAmount
        shiftAmount -= 1;
        carry = getBits(shiftAmount, shiftAmount, regFile[regM]);
    }

    return carry;
}

uint32_t applyOpcode(uint32_t *regFile, uint32_t opcode, uint32_t regN,
                     uint32_t op2)
{
    assert(opcode <= 13);
    
    switch(opcode) {
        case 0  : 
        case 8  : return regFile[regN] & op2;
        case 1  :
        case 9  : return regFile[regN] ^ op2;
        case 2  : 
        case 10 : return regFile[regN] - op2;
        case 3  : return op2 - regFile[regN];
        case 4  : return regFile[regN] + op2;
        case 12 : return regFile[regN] | op2;
        default : assert(opcode == 13);
                  return op2;
    } 
}

void setCPSRBit(uint32_t *CPSRreg, uint32_t bit, uint32_t value) {
    /* This function is called to set the specified bit of CPSR register
       to the given value */
    assert(bit <= 31 && bit >= 28);
    assert(value == 0 || value == 1);
   
    uint32_t mask = createMask(bit, bit);
 
    if(value == 0) {
        // invert the mask
        mask = ~mask;
    }    
    *CPSRreg &= mask;   
}       
