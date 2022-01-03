#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <stdio.h>
#include <bitset>

//this class hpp does a lot of including and lists down what exactly I need for a 5 stage pipeline
class CPU
{
public:
	int imem[256]; //instruction memory thats 32 bits per line/cell
	int reg_file[32]; //our register file, 32 bits per cell (32)
	uint8_t data_mem[1024]; //our data memory file, 8 bits per cell, or 1 byte per

	//this is what we extract from the instruction: opcode, funct3, rd, and rs1
	//for sw, rd represents imm[4:0]
	//the addi/ori/andi and lw instructions use the big imm worth 12 bits
	//add/sub/or/and and sw uses funct7/imm (take up the same place of 7 bits) and rs2
	int opcode;
	int funct3;
	int rs1;
	int rd; //for sw, represents imm[4:0]
	int rs2; // = 0 when we use addi, ori, andi, or lw
	uint64_t imm; //represents imm[11:0]
	int funct7_imm; //represents imm[11:5] for sw

	int regWrite; //1 if opcode is 0110011 or 0010011 or lw opcode, otherwise 0

	/*
	//control signals that still need to be implemented
	int aluSrc; //0 if we use rs2, 1 if we use immm
	int aluOp; //00 if sw/lw, 1 if branch, 1 rtype or some other shit idk
	int aluC; //
	int memRead; //1 if we lw
	int memWrite; //1 if we sw
	int memtoReg; //1 if we lw
	*/

	//retrieves the register file's value
	int get_reg_file(int);

	//writes to the register file
	void write_reg_file(int, int);

	//creates the instruction memory to be stored in myCPU
	void create_instr_mem(int input[256]);

	//creates a blank register file in myCPU
	void initialize_reg_file();

	//fetches a 32 bit value from instruction memory
	int Fetch(int);

	//interprets a 32 bit instruction, and then load the variables in myCPU
	int Decode(uint32_t);

	//resets all the values
	//currently unused
	void reset_code();

	//clears all values inside the data_memory
	void clear_data_memory();
};

//create the instruction memory, which for now is 256 cells of 32 bits: 256 cells of 4 bytes
//everything that isn't an instruction is 0
void CPU::create_instr_mem(int x[256])
{
	for (int i = 0; i < 256; i++)
	{
		imem[i] = x[i];
	}
}


//initialize the 32 celled 32-bit register file
void CPU::initialize_reg_file()
{
	for (int i = 0; i < 32; i++)
	{
		reg_file[i] = 0;
	}
}

//fetch the instruction from instruction memory
int CPU::Fetch(int i)
{
	return imem[i];
}

/*
decode the instruction
i.e separate the instruction into opcode, rd, funct3, rs1, rs2, funct7_imm, imm
based on that, we also change the control values of regWrite
we separate them by right shifting then & their values so that only the bits we need can have a 1 or 0
uint32_t instruction: what the current instruction is

TODO: put in control values of ALU, memWrite, etc.
*/
int CPU::Decode(uint32_t instruction)
{
	//since all commands use opcode, rd, rs1, and funct3, it can be shared
	opcode = instruction & 127;
	rd = (instruction >> 7) & 31;
	funct3 = (instruction >> 12) & 7;
	rs1 = (instruction >> 15) & 31;

	//we change the values of rs2, funct7_imm, imm, and regWrite depending on what the opcode is
	//if the value is 0, that means its not used
	//it shouldn't matter, since we are basing how our cpu works on opcode
	//if the opcode = 51, i.e 0110011, then its arithmetic operators that use rs2
	if (opcode == 51)
	{
		rs2 = (instruction >> 20) & 31;
		funct7_imm = (instruction >> 25) & 127;
		imm = 0;
		regWrite = 1;
	}
	//for r-type, this is when opcode = 0010011, or 19, is when we use immediates
	else if(opcode == 19)
	{
		rs2 = 0;
		funct7_imm = 0;
		imm = (instruction >> 20) & 4095;
		regWrite = 1;
	}
	//for opcode == 011 it means lw
	//only lw uses the big 12 bit immediate
	else if (opcode == 3)
	{
		rs2 = 0;
		funct7_imm = 0;
		imm = (instruction >> 20) & 4095;
		regWrite = 1;
	}
	//assumption here is if that the first three opcodes aren't found, then the last possible thing it could be is sw; the opcode = 0100011 btw
	else
	{
		rs2 = (instruction >> 20) & 31;
		funct7_imm = (instruction >> 25) & 127;
		imm = 0;
		regWrite = 0;
	}
	return 1;
}

//Resets everything in the CPU
//still unused
void CPU::reset_code()
{
	opcode = 0;
	rd = 0;
	funct3 = 0;
	rs1 = 0;
	rs2 = 0;
	funct7_imm = 0;
	imm = 0;
}

//retrieves the value in that register file
//place: the location to get the value from the register file
int CPU::get_reg_file(int place)
{
	return reg_file[place];
}


void CPU::write_reg_file(int place, int value)
{
	reg_file[place] = value;
}

void CPU::clear_data_memory()
{
	for (int i = 0; i < 1024; i++)
	{
		data_mem[i] = 0;
	}
}