#include <stdio.h>
#include <stdint.h> 

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
       
        // The new N and Z flag bit values        
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
    uint32_t regN = getBits(15, 12, instr); 
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

int main(void) { 
    return 0; 
} 
