#include "Core.h"

Core *initCore(Instruction_Memory *i_mem)
{
    Core *core = (Core *)malloc(sizeof(Core));
    core->clk = 0;
    core->PC = 0;
    core->instr_mem = i_mem;
    core->tick = tickFunc;
	 core->xregisters[0] = 0;

    return core;
}

// FIXME, implement this function
bool tickFunc(Core *core)
{
   // Steps may include
   // (Step 1) Reading instruction from instruction memory
   unsigned instruction = core->instr_mem->instructions[core->PC / 4].instruction;
   
   // (Step 2) ...
   printf("Instruction: %u -- %x\n", instruction, instruction);
	
	int next = decodeAndExecute(instruction); 

   // (Step N) Increment PC. FIXME, is it correct to always increment PC by 4?!
   core->PC += next;

   ++core->clk;
   // Are we reaching the final instruction?
   if (core->PC > core->instr_mem->last->addr)
   {
       return false;
   }
   return true;
}

void printRegisters(Core *core) {
	int i;
	for(i = 0; i < 32; i++) {
		printf("x%d: %d\n", i, core->xregisters[i]);
	}
}

void printMemory(Core *core) {
	int i;
	for(i = 0; i < 1024; i+=8) {
		printf("%d: %d|%d|%d|%d|%d|%d|%d|%d\n", i, core->xregisters[i], core->xregisters[i+1], core->xregisters[i+2], core->xregisters[i+3], core->xregisters[i+4], core->xregisters[i+5], core->xregisters[i+6], core->xregisters[i+7]);
	}
}

int decodeAndExecute(unsigned instruction) {
	unsigned opcode, rd, funct3, rs1, rs2, funct7, imm;
	int PC = 0;

	opcode = instruction & 127;
	if(opcode == 51){ // R type instruction
		rd = (instruction >> 7) & 31;
		funct3 = (instruction >> 7+5) & 7;
		rs1 = (instruction >> 7+5+3) & 31;
		rs2 = (instruction >> 7+5+3+5) & 31;
		funct7 = (instruction >> 7+5+3+5+5) & 127;

	} else if(opcode == 19) { // I Type instruction
		rd = (instruction >> 7) & 31;
		funct3 = (instruction >> 7+5) & 7;
		rs1 = (instruction >> 7+5+3) & 31;
		imm = (instruction >> 7+5+3+5) & 4095;
	
	} else if(opcode == 3) { // ld instruction
		rd = (instruction >> 7) & 31;
		imm = (instruction >> 7+5) & 7;
		rs1 = (instruction >> 7+5+3) & 31;
		rs2 = (instruction >> 7+5+3+5) & 31;
		funct7 = (instruction >> 7+5+3+5+5) & 127;
	
	} else if(opcode == 103) { // jalr instruction 
		rd = (instruction >> 7) & 31;
		funct3 = (instruction >> 7+5) & 7;
		rs1 = (instruction >> 7+5+3) & 31;
		rs2 = (instruction >> 7+5+3+5) & 31;
		funct7 = (instruction >> 7+5+3+5+5) & 127;
	
	} else if(opcode == 35) { // sd instruction
		rd = (instruction >> 7) & 31;
		funct3 = (instruction >> 7+5) & 7;
		rs1 = (instruction >> 7+5+3) & 31;
		rs2 = (instruction >> 7+5+3+5) & 31;
		funct7 = (instruction >> 7+5+3+5+5) & 127;

	} else if(opcode == 99) { // SB type instruction
		rd = (instruction >> 7) & 31;
		funct3 = (instruction >> 7+5) & 7;
		rs1 = (instruction >> 7+5+3) & 31;
		rs2 = (instruction >> 7+5+3+5) & 31;
		funct7 = (instruction >> 7+5+3+5+5) & 127;

	} else if(opcode == 111) { // jal instruction
		rd = (instruction >> 7) & 31;
		funct3 = (instruction >> 7+5) & 7;
		rs1 = (instruction >> 7+5+3) & 31;
		rs2 = (instruction >> 7+5+3+5) & 31;
		funct7 = (instruction >> 7+5+3+5+5) & 127;

	} else {
		printf("OP code undefined: %X",  opcode);
		PC = 4;
	}
	return PC;
}

uint64_t generateImmediateI(unsigned num) {	
	return (uint64_t) num;
}

uint64_t ALU(uint64_t data1, uint64_t data2, unsigned control) {
	switch(control) {
		case 1: // add
			return data1 + data2;
		case 2: // sub
			return data1 - data2;
		case 3: // and
			return data1 & data2;
		case 4: // or
			return data1 | data2;
		case 5: // xor
			return data1 ^ data2;
		case 6: // sll
			return data1 << data2;
		case 7: // srl
			return data1 >> data2;
		case 8: // eq
			return data1 == data2;
		case 9: // lt
			return data1 < data2;
		case 10: // gt
			return data1 > data2;
		default:
			printf("Unknown ALU command: %d\n", control);
	}
}

bool storeDouble(Core *core, int addr, uint64_t data) {
	uint8_t val;
	int i;
	if((addr < 0 || addr > 31) && addr % 8 == 0)
		return false;
	for(i = 0; i < 8; i++) {
		val = data >> (8*i);
		core->memory[addr + i] = val;
	}
	return true;
}

bool loadDouble(Core *core, int reg, int addr) {
	uint64_t val;
	int i;
	if((addr < 0 || addr > 31) && addr % 8 == 0)
		return false;
	for(i = 7; i >= 0; i--) {
		val += core->memory[addr + i];
		val = val << 8;
	}
	core-xregisters[reg] = val;
}

uint64_t getRegister(Core *core, int reg) {
	if(reg < 0 || reg > 31)
		return false;
	return core->xregisters[reg];
}
