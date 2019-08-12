#include "Parser.h"

// FIXME, implement this function.
// Here shows an example on how to translate "add x10, x10, x25"
void loadInstructions(Instruction_Memory *i_mem, const char *trace)
{
    printf("Loading trace file: %s\n", trace);

    FILE *fd = fopen(trace, "r");
    if (fd == NULL)
    {
        perror("Cannot open trace file. \n");
        exit(EXIT_FAILURE);
    }

    // Iterate all the assembly instructions
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    Addr PC = 0; // program counter points to the zeroth location initially.
    int IMEM_index = 0;
    while ((read = getline(&line, &len, fd)) != -1)
    {
        // Assign program counter
        i_mem->instructions[IMEM_index].addr = PC;

        // Extract operation
        char *raw_instr = strtok(line, " ");
        if (strcmp(raw_instr, "add") == 0 ||
            strcmp(raw_instr, "sub") == 0 ||
            strcmp(raw_instr, "sll") == 0 ||
            strcmp(raw_instr, "srl") == 0 ||
            strcmp(raw_instr, "xor") == 0 ||
            strcmp(raw_instr, "or")  == 0 ||
            strcmp(raw_instr, "and") == 0) {
            parseRType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
		  } else if(
		      strcmp(raw_instr, "addi") == 0 ||
            strcmp(raw_instr, "slli") == 0 ||
            strcmp(raw_instr, "srli") == 0 ||
            strcmp(raw_instr, "jalr") == 0 ||
            strcmp(raw_instr, "xori") == 0 ||
            strcmp(raw_instr, "ori")  == 0 ||
            strcmp(raw_instr, "andi") == 0 ||
				strcmp(raw_instr, "ld") == 0) {
				parseIType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
		  } else if(
				strcmp(raw_instr, "sd") == 0) {
				parseSType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
		  } else if(
		      strcmp(raw_instr, "beq") == 0 ||
            strcmp(raw_instr, "bne") == 0 ||
            strcmp(raw_instr, "blt") == 0 ||
				strcmp(raw_instr, "bge") == 0) {
				parseSBType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
		  } else if(
		      strcmp(raw_instr, "jal") == 0) {
				parseUJType(raw_instr, &(i_mem->instructions[IMEM_index]));
            i_mem->last = &(i_mem->instructions[IMEM_index]);
		  } 

        IMEM_index++;
        PC += 4;
    }

    fclose(fd);
}

void parseRType(char *opr, Instruction *instr)
{
    instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned funct7 = 0;

    if (strcmp(opr, "add") == 0) {
        opcode = 51;
        funct3 = 0;
        funct7 = 0;
    } else if (strcmp(opr, "sub") == 0) {
        opcode = 51;
        funct3 = 0;
        funct7 = 32;
    } else if (strcmp(opr, "sll") == 0) {
        opcode = 51;
        funct3 = 1;
        funct7 = 0;
    } else if (strcmp(opr, "srl") == 0) {
        opcode = 51;
        funct3 = 5;
        funct7 = 0;
    } else if (strcmp(opr, "xor") == 0) {
        opcode = 51;
        funct3 = 4;
        funct7 = 0;
    } else if (strcmp(opr, "or") == 0) {
        opcode = 51;
        funct3 = 6;
        funct7 = 0;
    } else if (strcmp(opr, "and") == 0) {
        opcode = 51;
        funct3 = 7;
        funct7 = 0;
    }

    char *reg = strtok(NULL, ", ");
    unsigned rd = regIndex(reg);

    reg = strtok(NULL, ", ");
    unsigned rs_1 = regIndex(reg);

    reg = strtok(NULL, ", ");
    reg[strlen(reg)-1] = '\0';
    unsigned rs_2 = regIndex(reg);

    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
    instr->instruction |= (funct7 << (7 + 5 + 3 + 5 + 5));
}

void parseIType(char *opr, Instruction *instr) {
	instr->instruction = 0;
   unsigned opcode = 0;
	unsigned funct3 = 0;

   if (strcmp(opr, "addi") == 0) {
       opcode = 19;
       funct3 = 0;
   } else if (strcmp(opr, "slli") == 0) {
       opcode = 19;
       funct3 = 1;
   } else if (strcmp(opr, "srli") == 0) {
       opcode = 19;
       funct3 = 5;
   } else if (strcmp(opr, "xori") == 0) {
       opcode = 19;
       funct3 = 4;
   } else if (strcmp(opr, "andi") == 0) {
       opcode = 19;
       funct3 = 7;
   } else if (strcmp(opr, "ori") == 0) {
       opcode = 19;
       funct3 = 6;
   } else if (strcmp(opr, "ld") == 0) {
       opcode = 3;
       funct3 = 3;
   } else if (strcmp(opr, "jalr") == 0) {
       opcode = 103;
       funct3 = 0;
   } 
   char *reg = strtok(NULL, ", "), *ld;
   unsigned rd = regIndex(reg);

	unsigned rs_1, immediate;
	if(strcmp(opr, "ld") == 0 || strcmp(opr,"jalr") == 0) {
		ld = strtok(NULL, "(");
		immediate = atoi(ld);

		reg = strtok(NULL, ")");
		rs_1 = regIndex(reg);
	} else {
		reg = strtok(NULL, ", ");
   	rs_1 = regIndex(reg);
	
		reg = strtok(NULL, ", ");
   	reg[strlen(reg)-1] = '\0';
		immediate = atoi(reg);
	}

   // Contruct instruction
   instr->instruction |= opcode;
   instr->instruction |= (rd << 7);
   instr->instruction |= (funct3 << (7 + 5));
   instr->instruction |= (rs_1 << (7 + 5 + 3));
   instr->instruction |= (immediate << (7 + 5 + 3 + 5));
}

void parseSType(char *opr, Instruction *instr) {
	instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;

    if (strcmp(opr, "sd") == 0) {
        opcode = 35;
        funct3 = 3;
    }

    char *reg = strtok(NULL, ", "), *sd;
    unsigned rs_1 = regIndex(reg);
	 
	 sd = strtok(NULL, "(");
	 unsigned immediate = atoi(sd);

	 reg = strtok(NULL, ")");
	 unsigned rs_2 = regIndex(reg);

    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= ((immediate&31) << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
    instr->instruction |= (((immediate>>5)&127) << (7 + 5 + 3 + 5 + 5));
	 
}

void parseSBType(char *opr, Instruction *instr) {
	 instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned funct7 = 0;

    if (strcmp(opr, "beq") == 0) {
        opcode = 99;
        funct3 = 0;
        funct7 = 0;
    } else if (strcmp(opr, "bne") == 0) {
        opcode = 99;
        funct3 = 1;
        funct7 = 0;
    } else if (strcmp(opr, "blt") == 0) {
        opcode = 99;
        funct3 = 4;
        funct7 = 0;
    } else if (strcmp(opr, "bge") == 0) {
        opcode = 99;
        funct3 = 5;
        funct7 = 0;
    }

    char *reg = strtok(NULL, ", ");
    unsigned rs_1 = regIndex(reg);

    reg = strtok(NULL, ", ");
    unsigned rs_2 = regIndex(reg);

    reg = strtok(NULL, ", ");
    reg[strlen(reg)-1] = '\0';
    unsigned immediate = atoi(reg);

    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= (((immediate >> 11)&1) << 7);
    instr->instruction |= (((immediate >> 1)&15) << 7+1);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
    instr->instruction |= (((immediate>>5)&63) << (7 + 5 + 3 + 5 + 5));
    instr->instruction |= (((immediate>>12)&1) << (7 + 5 + 3 + 5 + 5 + 6));

}

void parseUJType(char *opr, Instruction *instr) {
	instr->instruction = 0;
    unsigned opcode = 0;
    unsigned funct3 = 0;
    unsigned funct7 = 0;

    if (strcmp(opr, "jal") == 0) {
        opcode = 111;
        funct3 = 0;
        funct7 = 0;
    }

    char *reg = strtok(NULL, ", ");
    unsigned rd = regIndex(reg);

    reg = strtok(NULL, ", ");
    reg[strlen(reg)-1] = '\0';
    unsigned immediate = atoi(reg);

    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (((immediate>>12)&255) << (7 + 5));
    instr->instruction |= (((immediate>>11)&1) << (7 + 5 + 8));
    instr->instruction |= (((immediate>>1)&1023) << (7 + 5 + 8 + 1));
    instr->instruction |= (((immediate>>20)&1) << (7 + 5 + 8 + 1 + 10));

}

int regIndex(char *reg)
{
    unsigned i = 0;
    for (i; i < NUM_OF_REGS; i++) {
        if (strcmp(REGISTER_NAME[i], reg) == 0) {
            break;
        }
    }

    return i;
}
