#include "myCPU.hpp"
/* 
Add all the required standard and developed libraries here. 
Remember to include all those files when you are submitting the project. 
*/

/*Notes for myself
Start with the r-type text shit bc its the easiest
then LW/SW instructions
finally Test instruction bc why not

When there is no more input, i.e 5x of opcode = 0, the program will stop


*/

/*
Put/Define any helper function/definitions you need here
*/

//the structure for the instruction stage
//we get the instruction from instruction memory
struct instructed
{
	int bit_instruction;
	uint32_t opcode;
};

//the structure for the decode stage
//here we get the instruction from instruction stage and then cut it up into its parts, for rs1, funct3, opcode, etc.
//we also get the control signals, which is just regWrite
//since its the opcode (and funct3 to a lesser extent) that determines what happens
struct decoded
{
	uint32_t rs1_val;
	uint32_t rs2_val;
	uint64_t imm;
	uint32_t opcode;
	uint32_t funct3;
	uint32_t funct7_imm;
	uint32_t rd;

	int regWrite;
};

//executes the instructions that were decoded
//here we do the arithmetic for arithmetic instructions
// we also need to transfer everything from decode to memorize stage incase the opcode demands a lw/sw
// the result is value stored after doing arithmetic
struct executed
{
	uint32_t result;
	uint32_t rd;
	uint32_t opcode;
	uint32_t funct7_imm;
	uint32_t imm;
	uint32_t rs1_val;
	uint32_t rs2_val;
	int regWrite;
};

//executes the instructions that weren't done in the execution stage, i.e memory modification
//here we do the instructions for lw and sw
//lw alsu uses the result variable for what is being taken out of the memory file
//this is also where sw is done

struct memorized
{
	uint32_t rs1_val;
	uint32_t rs2_val;
	uint32_t result;
	uint32_t rd;
	int regWrite;
	uint32_t opcode;
	uint32_t funct7_imm;
	uint32_t imm;
};

//the write back stage, where we write back to the register 
//we write back to the register iff the control signal regWrite = 1
struct wrote_back
{
	uint32_t result;
	uint32_t rd;
	int regWrite;
	int opcode;
};

int main(int argc, char* argv[]) // your project should be executed like this: ./cpusim filename.txt and should print (a0,a1) 
{
	/* This is the front end of your project.
	You need to first read the instructions that are stored in a file and load them into an instruction memory.
	*/
	
	//reads the file
	//idk what else to say
	std::ifstream r;
	r.open(argv[1]);

	/* Define your Instruction memory here. Each cell should store 1 byte. You can define the memory either dynamically, or define it as a fixed size with size 4MB (i.e., 4096 lines). Each instruction is 32 bits (i.e., 4 lines, saved in little-endian mode).
	Each line in the input file is stored as an unsigned char and is 1 byte (each four lines are one instruction). You need to read the file line by line and store it into the memory. You may need a mechanism to convert these values to bits so that you can read opcodes, operands, etc.
	*/

	//This is the instruction memory
	//it holds 1-24 lines, which are inputted from the r.open() thing and the textfile
	//it parses through the text file, converts them into ints, and then inputs them into our instr memory
	//initially initalize our memory with 0's
	int instMem[1024] = { 0 };


	//parses through the text file, and fill our instruction memory array with the input of the text file
	//converts the string into integers btw
	int counter = 0;
	std::string temp;
	while (getline(r, temp))
	{
		instMem[counter] = std::stoi(temp, nullptr, 10);
		counter++;
	}

	//create the instruction matrix early on
	//256 cells because 1024/4 = 256
	int instr[256];

	//a for loop that parses through every 4 cells of our memory, i.e every 32 bits or every 4 bytes
	//since little endian, the last byte should be the first in terms of bitwise representation, hence left shift by 24
	//the first doesn't get left shift, etc.
	for (int j = 0; j < 256; j++)
	{
		int first = instMem[j * 4];
		int second = instMem[j * 4 + 1] << 8;
		int third = instMem[j * 4 + 2] << 16;
		int fourth = instMem[j * 4 + 3] << 24;

		//add up the total values, which gets the value we need to convert into bit
		int total = first + second + third + fourth;

		//now all the 32 bitlength instructions are in the instruction matrix, represented as an int
		instr[j] = total;
	}

	//close the file bc good manners (:
	r.close();


	/* OPTIONAL: Instantiate your Instruction object here. */
	//to access each element of the instruction matrix, use a counter, and increase the counter if x happens pretty much
	//since the instruction matrix has string elements, we can use substrings to cut it up and do x with each element

	/* OPTIONAL: Instantiate your CPU object here. */
	//creates the instruction memory in the CPU as well as initialize the register file
	CPU myCPU;
	myCPU.create_instr_mem(instr);
	myCPU.initialize_reg_file();

	// Clock and PC
	//uint_64 myClock = 0; // data-types can be changed! This is just a suggestion. 
	//uint_64 myPC = 0; 
	//unsure about what this does; need to check with teacher later

	int myClock = 0;
	int myPC = 0;

	//initialize everything we're planning to use
	// current instruction, next, etc.
	instructed curr_instr, next_instr;
	decoded curr_decode, next_decode;
	executed curr_exe, next_exe;
	memorized curr_mem, next_mem;
	wrote_back curr_wb;

	//Reminder: its a 5 stage processor
	//I need to use the next_values and all that
	//first step is making sure that one instruction can run through smoothly
	while (1) // processor's main loop. Each iteration is equal to one clock cycle.  
	{
		//for some reason adding another matrix would change the value of x0
		//need to ask the professor about this later on
		//at least this way the value of x0 would never change
		myCPU.write_reg_file(0, 0);

		//fetch stage
		//This is when we get the the instruction from instruction memory
		// the instruction is a 32 bitlength string function
		//... = myCPU.Fetch(...) // fetching the instruction

		//depending on the value of myClock, it fetches the instruction from the instruction memory
		//we also extract the opcode at this point to see if it has 0's or not
		curr_instr.bit_instruction = myCPU.Fetch(myClock);
		curr_instr.opcode = curr_instr.bit_instruction & 127;

		//The decode stage
		//we decode the instructions in the CPU class, and extract all relevant variables
		// if there is a variable not used, ex: funct7_imm for andi instructions, it's equivalent to 0
		//

		myCPU.Decode(next_instr.bit_instruction);

		curr_decode.opcode = myCPU.opcode;
		curr_decode.rs1_val = myCPU.get_reg_file(myCPU.rs1);
		curr_decode.rs2_val = myCPU.get_reg_file(myCPU.rs2);
		curr_decode.rd = myCPU.rd;
		curr_decode.funct3 = myCPU.funct3;
		curr_decode.funct7_imm = myCPU.funct7_imm;
		curr_decode.imm = myCPU.imm;
		curr_decode.regWrite = myCPU.regWrite;

		// execute
		// the execute stage actually does the math and stuff
		// ALU, shift left1, add, ALU control ofc
		// What execute does depends on what the opcode, funct3, and what the first 11 values or whatever of the bit thint is
		// 

		//If there is time, change the bottom if/else statements to use ALUOp and ALU control input

		//if the opcode is 19, that means its the alu using immediates
		//for that we use rs1, rd, and the immediate

		//if the opcode is 19, that means its the instruction that use immediates, i.e addi, andi, and ori
		//that means we use the values for rs1 and imm
		//we also use funct3 to determine exactly which of the three instructions to use
		if (next_decode.opcode == 19)
		{
			uint64_t temp_result;
			//if the funct3 = 0 its an addi instruction
			if (next_decode.funct3 == 0)
			{
				temp_result = next_decode.rs1_val + next_decode.imm;
			}
			//if funct3 = 6, its an ori instruction
			else if (next_decode.funct3 == 6)
			{
				temp_result = next_decode.rs1_val | next_decode.imm;
			}
			//assume nothing else can happen, therefore its andi
			else
			{
				temp_result = next_decode.rs1_val & next_decode.imm;
			}
			//save the results and register destination here
			curr_exe.result = temp_result;
			curr_exe.rd = next_decode.rd;
		}
		//opcode is 51, then the alu uses 2 registers for and, add, sub, and or
		// again, funct3 determines what we use, as well as funct7_imm that determines if add or sub is used
		//now we use funct7, rs2, rs1, funct3
		else if (next_decode.opcode == 51)
		{
			uint64_t temp_result;
			if (next_decode.funct3 == 0)
			{
				//determine to either add or sub, depending on funct7_imm
				if (next_decode.funct7_imm == 0)
				{
					temp_result = next_decode.rs1_val + next_decode.rs2_val;
				}
				//assume if funct7_imm != 0, that means it will be 0100000
				else
				{
					temp_result = next_decode.rs1_val - next_decode.rs2_val;
				}
			}
			//determines what to do for or or &
			//uses the same thought process for the ori and andi instruction
			else if (next_decode.funct3 == 6)
			{
				temp_result = next_decode.rs1_val | next_decode.rs2_val;
			}
			else
			{
				temp_result = next_decode.rs1_val & next_decode.rs2_val;
			}
			//stores the result and register destination
			curr_exe.result = temp_result;
			curr_exe.rd = next_decode.rd;
		}
		//this is when the opcode is either lw or sw; since they don't need any arithmetic expressions
		//and alusrc is integrate with other stuff for now ):
		else
		{
			curr_exe.rs1_val = next_decode.rs1_val;
			curr_exe.rs2_val = next_decode.rs2_val;
			curr_exe.result = 0;
			curr_exe.rd = next_decode.rd;
		}

		//this should be transfered over to the next stage no matter what the opcode is
		//control signal regWrite, the opcode, funct7_imm, and imm for loadword
		curr_exe.regWrite = next_decode.regWrite;
		curr_exe.opcode = next_decode.opcode;
		curr_exe.funct7_imm = next_decode.funct7_imm;
		curr_exe.imm = next_decode.imm;

		// memory
		// this writes to the data memory if need to, for lw and sw i guess
		//

		//when opcode = 3, this is a loadword
		// a loadword uses imm[11:0] = imm, rs1, and rd
		//we also need to retrieve the data from the memory
		//we get the data from each of the memory file, left shift them depending on where they were got (0th place doesn't left shift, 4th place left shifts 6x)
		//we then add them all up and store the total into the result
		//we also transfer over the register destination because we need to do that
		if (next_exe.opcode == 3)
		{
			uint32_t total = 0;
			uint32_t o_location = next_exe.rs1_val;
			uint32_t offset = next_exe.imm;
			for (int j = 0; j < 4; j++)
			{
				total += (myCPU.data_mem[o_location + offset + j]) << (j*2);
			}
			curr_mem.result = total;
			curr_mem.rd = next_exe.rd;
		}
		//when opcode == 35, this is a storeword
		// a storeword uses imm[11:5] = funct7_imm and imm[4:0] = rd, rs1, and rs2
		//we divy up the data we need to place into 4 different bytes, right shift and and them
		// and then send the pieces to the memory file
		//thats about it 
		else if (next_exe.opcode == 35)
		{
			uint32_t o_location = next_exe.rs1_val;
			uint32_t get_input = next_exe.rs2_val;
			uint32_t offset_1 = next_exe.rd;
			uint32_t offset_2 = next_exe.funct7_imm << 5;
			uint32_t offset = offset_1 + offset_2;

			for (int j = 0; j < 4; j++)
			{
				myCPU.data_mem[o_location + offset + j] = ((get_input >> (j * 2)) & 3);
			}
		}
		//if its an arithmetic instruction, we just transfer the result and rd over
		else
		{
			curr_mem.result = next_exe.result;
			curr_mem.rd = next_exe.rd;
		}
		//no matter the instruction, we transfer over the regWrite and opcode variables over
		curr_mem.opcode = next_exe.opcode;
		curr_mem.regWrite = next_exe.regWrite;

		// writeback
		// we send that shit straight back to the register file and write it there
		// of course, that's only if the opcode/writetoReg demands it so demands it so
		//there is no next_write back stage, since there isn't a pipeline for it
		curr_wb.result = next_mem.result;
		curr_wb.rd = next_mem.rd;
		curr_wb.regWrite = next_mem.regWrite;
		curr_wb.opcode = next_mem.opcode;

		//this only happens for the arithmetic functions and load word
		if (curr_wb.regWrite == 1)
		{
			//std::cout << curr_wb.result << std::endl;
			myCPU.write_reg_file(curr_wb.rd, curr_wb.result);
		}

		// _next values should be written to _current values here:
		//
		// There should be at least 2 different values: current and next

		next_instr = curr_instr;
		next_decode = curr_decode;
		next_exe = curr_exe;
		next_mem = curr_mem;

		//add the clock and pc values up
		myClock += 1;
		myPC += 4; //for now we can assume that the next PC is always PC + 4

		//we should break the loop if ALL instructions in the pipeline has opcode == 0 instruction
		//i.e. if all curr_... opcodes are all 0

		if (curr_instr.opcode == 0 && curr_decode.opcode == 0 && curr_exe.opcode == 0 && curr_mem.opcode == 0 && curr_wb.opcode == 0)
		{
			//this prints out the final value of the PC so we can find IPC
			//std::cout << myPC << std::endl;

			//this prints out myClock, i.e the total amount of clock cycles
			//std::cout << myClock << std::endl;
			break;
		}
	}
	//clean up the memory (if any)
	myCPU.clear_data_memory();

	for (int i = 0; i < 32; i++)
	{
		std::cout << "The register value of x" << i << " is " << myCPU.get_reg_file(i) << std::endl;
	}

	std::cout << "(" << myCPU.get_reg_file(10) << ", " << myCPU.get_reg_file(11) << ")";
	return 0;
}