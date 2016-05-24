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
                                                                                       
// Created a global pointer to access main memory and reg file                         
static uint8_t *mainMem;                                                               
static uint32_t *regFile;                                                              
                                                                                       
struct state {                                                                         
    uint32_t decoded;                                                                  
    uint32_t fetched;                                                                  
};                                                                                     
                                                                                       
typedef enum instructionType {                                                         
    DP = 0, MUL = 1, SDT = 2, BR = 3                                                   
} Instruction;                                                                         
                                                                                       
// Functions used to check condition                                                   
int checkCond(uint32_t cond);                                                          
int checkZ(void);                                                                      
int equalityNV(void);                                                                  
                                                                                       
// Branch functions                                                                    
int branch(uint32_t instr);                                                            
                                                                                       
// Multiply functions                                                                  
void mult(uint32_t instr);                                                             
uint32_t multInstr(uint32_t instr);                                                    
uint32_t accMultInstr(uint32_t instr);                                                 
                                                                                       
// Main utility functions                                                              
Instruction checkInstruction(uint32_t instr);                                          
Instruction checkCaseOne(uint32_t instr);                                              
Instruction checkCaseTwo(uint32_t instr);                                              
void printState(void);                                                                 
void simulatePipeline(struct state *ARMState);                                         
                                                                                       
// Single Data Transfer functions                                                      
void singleDataTransfer(uint32_t instr);                                               
uint32_t logicalLeftShift(uint32_t amount, uint32_t value);                            
uint32_t logicalRightShift(uint32_t amount, uint32_t value);                           
uint32_t arithmeticRightShift(uint32_t amount, uint32_t value);                        
uint32_t rotateRight(uint32_t amount, uint32_t value);            

// Data Proccessing Instructions                                                       
void dataProcessInstr(uint32_t instr);                                                 
uint32_t evaluateShiftedReg(uint32_t operand);                                         
uint32_t evaluateImmediateValue(uint32_t operand);                                     
uint32_t executeOpcode(uint32_t opcode, uint32_t regN, uint32_t op2);                  
uint32_t getCarryFromShifter(uint32_t op2);                                            
uint32_t isOverflow(uint32_t int1, uint32_t int2);                                     
uint32_t getCarryFromALU(uint32_t instr, uint32_t op2);                                
                                                                                       
// Utility functions                                                                   
void setCPSRBit(uint32_t bit, uint32_t value);                                         
uint32_t getInteger(uint32_t firstByteAddr);                                           
uint32_t switchEndian(uint32_t num);                                                   
uint32_t getBits(int leftmost, int rightmost, uint32_t num);                           
uint32_t createMask(uint32_t top, uint32_t bot);                                       

