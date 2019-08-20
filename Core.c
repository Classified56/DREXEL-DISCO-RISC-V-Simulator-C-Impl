#include "Core.h"

Core *initCore(Instruction_Memory *i_mem)
{
	 int i;
    Core *core = (Core *)malloc(sizeof(Core));
    core->clk = 0;
    core->PC = 0;
    core->instr_mem = i_mem;
    core->tick = tickFunc;
	 for(i = 0; i < 32; i++)
	 	core->xregisters[i] = 0;
	 for(i = 0; i < 1024; i++)
		core->memory[i] = 0;
	
	 // Manual Initialization of Registers and Memory
	 core->xregisters[1]=7;
	 core->xregisters[3]=2;
	 core->xregisters[5]=5;
	 core->xregisters[6]=10;
	 core->memory[0]=16;
	 core->memory[1]=128;
	 core->memory[2]=8;
	 core->memory[3]=4;

	 printRegisters(core);
    return core;
}

// FIXME, implement this function
bool tickFunc(Core *core)
{
   // Steps may include
   // (Step 1) Reading instruction from instruction memory
   unsigned instruction = core->instr_mem->instructions[core->PC / 4].instruction;
   
   // (Step 2) ...
   //printf("Instruction: %u -- %x\n", instruction, instruction);
	
	int next = decodeAndExecute(core, instruction); 

   // (Step N) Increment PC. FIXME, is it correct to always increment PC by 4?!
   core->PC += next;

   ++core->clk;

	//printf("PC: %lu | CLK: %lu\n", core->PC, core->clk);
	
   // Are we reaching the final instruction?
   if (core->PC > core->instr_mem->last->addr)
   {
		printRegisters(core);
		printMemory(core);
      return false;
   }
   return true;
}

void printRegisters(Core *core) {
	int i;
	for(i = 0; i < 32; i++) {
		printf("x%d: %lu\n", i, core->xregisters[i]);
	}
}

void printMemory(Core *core) {
	int i;
	for(i = 0; i < 1024; i+=8) {
		printf("%d: %d|%d|%d|%d|%d|%d|%d|%d\n", i, core->memory[i], core->memory[i+1], core->memory[i+2], core->memory[i+3], core->memory[i+4], core->memory[i+5], core->memory[i+6], core->memory[i+7]);
	}
}

int decodeAndExecute(Core *core, unsigned instruction) {
	unsigned opcode, rd, funct3, rs1, rs2, funct7, imm;
	uint64_t data1, data2, result;
	int PC = 4;

	opcode = instruction & 127;
	if(opcode == 51){ // R type instruction
		rd = (instruction >> 7) & 31;
		funct3 = (instruction >> 7+5) & 7;
		rs1 = (instruction >> 7+5+3) & 31;
		rs2 = (instruction >> 7+5+3+5) & 31;
		funct7 = (instruction >> 7+5+3+5+5) & 127;
		
		data1 = getRegister(core, rs1);
		data2 = getRegister(core, rs2);

		if(funct3 == 0) {
			if(funct7 == 0) {
				result = ALU(data1, data2, 1);
				//printf("ADD: %lu=%lu+%lu\n", result, data1, data2);
			} else if(funct7 == 32) {
				result = ALU(data1, data2, 2);
				//printf("SUB: %lu=%lu-%lu\n", result, data1, data2);
			}
		} else if(funct3 == 7) {
			result = ALU(data1, data2, 3);
			//printf("AND: %lu=%lu&%lu\n", result, data1, data2);
		} else if(funct3 == 6) {
			result = ALU(data1, data2, 4);
			//printf("OR: %lu=%lu|%lu\n", result, data1, data2);
		} else if(funct3 == 4) {
			result = ALU(data1, data2, 5);
			//printf("XOR: %lu=%lu^%lu\n", result, data1, data2);
		} else if(funct3 == 1) {
			result = ALU(data1, data2, 6);
			//printf("SLL: %lu=%lu<<%lu\n", result, data1, data2);
		} else if(funct3 == 5) {
			result = ALU(data1, data2, 7);
			//printf("SRL: %lu=%lu<<%lu\n", result, data1, data2);
		}
		loadData(core, rd, result);
	} else if(opcode == 19) { // I Type instruction
		rd = (instruction >> 7) & 31;
		funct3 = (instruction >> 7+5) & 7;
		rs1 = (instruction >> 7+5+3) & 31;
		imm = (instruction >> 7+5+3+5) & 4095;
		
		data1 = getRegister(core, rs1);
		data2 = generateImmediate(imm);

		if(funct3 == 0) {
			result = ALU(data1, data2, 1);
			//printf("ADDi: %lu=%lu+%lu\n", result, data1, data2);
		} else if(funct3 == 7) {
			result = ALU(data1, data2, 3);
			//printf("ANDi: %lu=%lu&%lu\n", result, data1, data2);
		} else if(funct3 == 6) {
			result = ALU(data1, data2, 4);
			//printf("ORi: %lu=%lu|%lu\n", result, data1, data2);
		} else if(funct3 == 4) {
			result = ALU(data1, data2, 5);
			//printf("XORi: %lu=%lu^%lu\n", result, data1, data2);
		} else if(funct3 == 1) {
			result = ALU(data1, data2, 6);
			//printf("SLLi: %lu=%lu<<%lu\n", result, data1, data2);
		} else if(funct3 == 5) {
			result = ALU(data1, data2, 7);
			//printf("SLRi: %lu=%lu>>%lu\n", result, data1, data2);
		}
		loadData(core, rd, result);
	} else if(opcode == 3) { // ld instruction
		rd = (instruction >> 7) & 31;
		funct3 = (instruction >> 7+5) & 7;
		rs1 = (instruction >> 7+5+3) & 31;
		imm = (instruction >> 7+5+3+5) & 4095;	
		
		data1 = getRegister(core, rs1);

		if(loadDouble(core, rd, data1+imm)) {
			result = getRegister(core, rd);
			//printf("LOAD: R[%u]=M[%u+%u]=%lu\n", rd, rs1, imm, result);
		}

	} else if(opcode == 103) { // jalr instruction 
		rd = (instruction >> 7) & 31;
		funct3 = (instruction >> 7+5) & 7;
		rs1 = (instruction >> 7+5+3) & 31;
		imm = (instruction >> 7+5+3+5) & 4095;
		
		data1 = getRegister(core, rs1);
		data2 = generateImmediate(imm);

		result = ALU(data1, data2, 1);
		
		loadData(core, rd, core->PC);
		PC = result;
		//printf("JALR: PC=%lu, R[%u]=%lu", result, rd, getRegister(core, rd));
	} else if(opcode == 35) { // sd instruction
		imm = (instruction >> 7) & 31;
		funct3 = (instruction >> 7+5) & 7;
		rs1 = (instruction >> 7+5+3) & 31;
		rs2 = (instruction >> 7+5+3+5) & 31;
		imm += ((instruction >> 7+5+3+5+5) & 127) << 5;
		
		data1 = getRegister(core, rs1);
		data2 = getRegister(core, rs2);
		if(storeDouble(core, data1+imm, data2)) {
			//printf("STORE: M[%u+%u]=R[%u]=%lu\n", rs1, imm, rs2, result);
		}

	} else if(opcode == 99) { // SB type instruction
		imm = (instruction >> 7) & 30;
		funct3 = (instruction >> 7+5) & 7;
		rs1 = (instruction >> 7+5+3) & 31;
		rs2 = (instruction >> 7+5+3+5) & 31;
		imm += ((instruction >> 7+5+3+5+5) & 63) << 5;
		imm += ((instruction >> 7) & 1) << 11;
		imm += ((instruction >> 7+5+3+5+5+6) & 1) << 12;
		
		data1 = getRegister(core, rs1);
		data2 = getRegister(core, rs2);
		
		if(funct3 == 0) {
			result = ALU(data1, data2, 8);
			//printf("BEQ: %lu==%lu : %lu\n", data1, data2, result);
			if(result){
				PC = imm;
			}
		} else if(funct3 == 1) {
			result = ALU(data1, data2, 8);
			//printf("BNE: %lu!=%lu : %lu\n", data1, data2, result);
			if(!result){
				PC = imm;
			}
		} else if(funct3 == 4) {
			result = ALU(data1, data2, 9);
			//printf("BLT: %lu<%lu : %lu\n", data1, data2, result);
			if(result == 0){
				PC = imm;
			}
		} else if(funct3 == 5) {
			result = ALU(data1, data2, 10);
			//printf("BGE: %lu>=%lu : %lu\n", data1, data2, result);
			if(result == 0){
				PC = imm;
			}
		}
	} else if(opcode == 111) { // jal instruction
		rd = (instruction >> 7) & 31;
		imm = (instruction >> 7+5) & 1048575;
		
		data1 = getRegister(core, rs1);
		data2 = generateImmediate(imm);

		result = ALU(data1, data2, 1);
		
		loadData(core, rd, core->PC);
		PC = result;
		//printf("JALR: PC=%lu, R[%u]=%lu\n", result, rd, getRegister(core, rd));

	} else {
		printf("OP code undefined: %X\n",  opcode);
	}
	//printf("PC = PC + %d\n", PC);
	return PC;
}

uint64_t generateImmediate(unsigned num) {	
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
			return data1 >= data2;
		default:
			printf("Unknown ALU command: %d\n", control);
			return -1;
	}
}

bool storeDouble(Core *core, int addr, uint64_t data) {
	uint8_t val;
	int i;
	if(addr < 0 || addr  > 1023) {
		printf("Store address out of bounds.\n");
		return false;
	}
	for(i = 0; i < 8; i++) {
		val = data >> (8*i);
		core->memory[addr + i] = val;
	}
	return true;
}

bool loadData(Core *core, int reg, uint64_t data) {
	int i;
	if(reg < 0 || reg > 31) {
		printf("Load address out of bounds.\n");
		return false;
	}
	core->xregisters[reg] = data;
	return true;
}

bool loadDouble(Core *core, int reg, int addr) {
	uint64_t val;
	int i;
	if(addr < 0 || addr > 1023 || reg < 0 || reg > 31) {
		printf("Load address out of bounds.\n");
		return false;
	}
	//printf("Load - reg: %d, addr: %d", reg, addr);

	for(i = 7; i >= 0; i--) {
		val += core->memory[addr + i];
		val = val << 8;
		//printf("Loading: val = %lu from %d\n", val, addr+i);
	}
	core->xregisters[reg] = val;
	return true;
}

uint64_t getRegister(Core *core, int reg) {
	if(reg < 0 || reg > 31){
		printf("Access address out of bounds.\n");
		return -1;
	}
	return core->xregisters[reg];
}
