#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

#define CPSR 16
#define PC 15
#define NUM_REG 17
#define GEN_REG 13
#define MAX_SIZE 65536

// It was suggested to us that we should remove the magic numbers by defining
// them as macros
#define MS_BIT 31
#define LSCOND_BIT 28
#define COND_Z_BIT 30
#define I_BIT 25
#define BR_MS_OFFSET_BIT 23
#define BR_LS_OPCODE_BIT 25
#define FST_B_LS_BIT 24
#define SND_B_MS_BIT 23
#define SND_B_LS_BIT 16
#define THRD_B_MS_BIT 15
#define THRD_B_LS_BIT 8
#define FRTH_B_MS_BIT 7
#define LS_BIT 0
#define SHFT_REG_KIND_BIT 4
#define SHFT_REG_CONST_MS_BIT 11
#define SHFT_REG_CONST_LS_BIT 7

struct state {
    uint32_t decoded;
    uint32_t fetched;
};

typedef enum instructionType {
    DP = 0, MUL = 1, SDT = 2, BR = 3
} Instruction;
             
                                  
// Main utility functions
//
//------------------------------------------------------------------

/* Determines the type of the instruction. Returns 0 if Data Processing, 
    1 if multiply, 2 if Single Data Transfer, and 3 if Branch. */ 
Instruction checkInstruction(uint32_t instr);

/* Determines whether the instruction is a Data Processing or Multiply
    instruction by checking specific bits */ 
Instruction checkCaseOne(uint32_t instr);

/* Determines whether the instruction is a Data Processing or Multiply
    instruction by checking specific bits */ 
Instruction checkCaseTwo(uint32_t instr);

/* Prints the registers and the non-zero memory addresses in the processor */
void printState(void);

/* Simulates the pipeline process which executes, decodes and fetches 
    instructions from the main memory */ 
void simulatePipeline(struct state *ARMState);

/* Frees all the memory allocated in the heap */
void freeAllMemory(struct state *ARMState);

// Data Proccessing Instructions
//
//------------------------------------------------------------------

/* This function is called by main. If the immediate operand is set
    then operand2 is an immediate value. If immediate operand is clear then
    operand2 is a register. Also sets flags depending on the S bit(bit 20)
    of the instruction. */
void dataProcessInstr(uint32_t instr);

/* Given an operand, outputs the value after the shifted register operation
    is applied to the value in Rm (bits 0 to 3) */
uint32_t evaluateShiftedReg(uint32_t operand);

/* This function is called by the data processing instruction if the
    immediate operand bit (bit 25) of the instruction is set. The function
    takes an 8 bit immediate value (bits 0 - 7) and rotates it by twice 
    the value of the rotate bits (bits 8-11). */
uint32_t evaluateImmediateValue(uint32_t operand);

/* Decodes opcode and applies it to Rn and op2 */
uint32_t executeOpcode(uint32_t opcode, uint32_t regN, uint32_t op2);

/* Gets carry from shifters when op2 is a register */
uint32_t getCarryFromShifter(uint32_t op2);

/* The carry is extracted from ALU operations */
uint32_t getCarryFromALU(uint32_t instr, uint32_t op2);

/* Checks if an overflow will occur if int1 and int2 are added together. */
uint32_t isOverflow(uint32_t int1, uint32_t int2);


// Multiply functions
//
//------------------------------------------------------------------

/* Multiplies the contents of two registers with the option to accumulate */
void mult(uint32_t instr);

/* Since the Accumulate bit is 0, then the multiply instruction is 
    carried out, and performs Rd:=RmxRs, where Rn is ignored */
uint32_t multInstr(uint32_t instr);

/* Since the Accumulate bit is 1, then the accumulate-multiply instruction is 
    carried out, and performs Rd:=RmxRs+Rn */
uint32_t accMultInstr(uint32_t instr);


// Single Data Transfer functions
//
//------------------------------------------------------------------

/* This function is used to do single data transfers. The instuction determines
    whether it is a load or a store and the offset for the base register */
void singleDataTransfer(uint32_t instr);

/* Given the operand determines the amount to be shifted, taking into account
    whether it is a constant amount or specified by a register */
uint32_t getShiftAmount(uint32_t operand);

/* Given the corresponding bit, it determines whether the function should load
    data from memory or store data into the memory give the dest/src reg */
void loadOrStore(uint32_t loadStoreBit, uint32_t reg, uint32_t index);

/* Helper function for single data transfer that stores the data from the source
    register to the main memory */
void storeData(uint32_t source, uint32_t index);

/* Helper function for single data transfer that loads the data from the main
    memory into the dest register */
void loadData(uint32_t dest, uint32_t index);


// Branch functions
//
//------------------------------------------------------------------

/* branch is a helper function that contains an 24 bit 2s complement
    offset that will be sign extended and added to "PC" */
uint32_t branch(uint32_t instr);


// Functions used to check conditions
//
//------------------------------------------------------------------

/* This function is used to check the condition of the instruction.
    It calls helper functions checkZ and equalityNV to return a value
    of 1 if the condition is met, or 0 if the condition is not met. */
uint32_t checkCond(uint32_t cond);

/* This function checks the 30th bit of the CPSR register (the Z
    bit) to see if it is set. Returns 1 if Z is set. Returns 0 if
    it is clear. */ 
uint32_t checkZ(void);

/* This function checks the 31st bit of the CPSR register (the N
    bit) and compares it with the 28th bit (the V bit). Returns 1 of
    N = C. Returns 0 if N != C */
uint32_t equalityNV(void);


// Shifted register helper functions
//
//------------------------------------------------------------------

/* Moves each bit to the left by the specified amount, extending with 0s */
uint32_t logicalLeftShift(uint32_t amount, uint32_t value);

/* Moves each bit to the right by the specified amount, extending with 0s */
uint32_t logicalRightShift(uint32_t amount, uint32_t value);

/* Makes an arithmetic shift, while preserving the sign bit */
uint32_t arithmeticRightShift(uint32_t amount, uint32_t value);

/* Rotates cyclically the value the amount number of bits */
uint32_t rotateRight(uint32_t amount, uint32_t value);


// Utility functions
//
//------------------------------------------------------------------

/* This function is called to set the specified bit of CPSR register
    to the given value */
void setCPSRBit(uint32_t bit, uint32_t value);

/* Extracts the first 4 bytes from the address and merges them into a 32 bit
    instruction in Big Endian */ 
uint32_t getInteger(uint32_t firstByteAddr);

/* Switches the endianness of a 32-bit int by swapping the positions of each
    byte */ 
uint32_t switchEndian(uint32_t num);

/* Selects the bits from a range when given a leftmost and a rightmost bit, 
    and returns the selected bits as a 32-bit int */ 
uint32_t getBits(uint32_t leftmost, uint32_t rightmost, uint32_t num);

/* createMask is a helper function that produces a mask (with correct
    offset) */
uint32_t createMask(uint32_t top, uint32_t bot);
