#ifndef __CORE_H__
#define __CORE_H__

#include "Instruction_Memory.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define BOOL bool

struct Core;
typedef struct Core Core;
typedef struct Core
{
    Tick clk; // Keep track of core clock
    Addr PC; // Keep track of program counter

    // TODO, define your components here
    uint64_t xregisters[32];
	 uint8_t memory[1024]; 

	 // What else you need? Data memory? Register file?
    Instruction_Memory *instr_mem;
    
    // TODO, simulation function
    bool (*tick)(Core *core);
}Core;

Core *initCore(Instruction_Memory *i_mem);
bool tickFunc(Core *core);
void printRegisters(Core *core);
void printMemory(Core *core);
int decodeAndExecute(Core *core, unsigned instruction);
uint64_t generateImmediate(unsigned num);
uint64_t ALU(uint64_t data1, uint64_t data2, unsigned control);
bool storeDouble(Core *core, int addr, uint64_t data);
bool loadDouble(Core *core, int reg, int addr);
bool loadData(Core *core, int reg, uint64_t data);
uint64_t getRegister(Core *core, int reg);

#endif
