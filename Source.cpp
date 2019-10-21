/*
References:
(1) The risc-v ISA Manual ver. 2.1 @ https://riscv.org/specifications/

x0  --> zero
x1 -->ra (return address)
x2 --> sp
x3 -->gp
x4 --> tp
x5-x7 --> t0-t2
x8 --> s0/fp
x9 --> s1
x10-x11 -->a0-a1 (function arguments/return values)
x12-x17 -->a2-a7 (for service numbers)
x18-x27 --> s2-s11
x28-x31 --> t3-t6
*/

#include <iostream>
#include <fstream>
#include "stdlib.h"
#include <iomanip>

using namespace std;

int regs[32] = { 0 }; //There are 32 registers
unsigned int pc = 0x0; // stores the hex value 0 in pc
char memory[8 * 1024];	// only 8KB of memory located at address 0

void emitError(const char* s)
{
	cout << s;
	exit(0);
}

void printPrefix(unsigned int instA, unsigned int instW) {
	cout << "0x" << hex << std::setfill('0') << std::setw(8) << instA << "\t0x" << std::setw(8) << instW;
}

void instDecExec(unsigned int instWord, bool& flag)
{
	regs[0] = 0;
	unsigned int rd, rs1, rs2, funct3, funct7, opcode;
	unsigned int I_imm, S_imm, B_imm, U_imm, J_imm;
	unsigned int address;
	int x, y, z;
	unsigned int instPC = pc - 4;
	opcode = instWord & 0x0000007F; // masking instWord with 7 bits (1s)   so it will be instaword & ..0000 1111111 then store the result in opcode
	rd = (instWord >> 7) & 0x0000001F; //masking (instWord>>7) with 5 bits(1s)   so it will be instaword  ...00 11111 ]then store the result in rd
	funct3 = (instWord >> 12) & 0x00000007;//masking (instWord>>12) with 3 bits (1s)   so it will be instaword & 111 then store the result in opcode
	rs1 = (instWord >> 15) & 0x0000001F; // masking(instWord >> 15) with 3 bits(1s)   so it will be instaword & 11111 then store the result in opcode
	rs2 = (instWord >> 20) & 0x0000001F; // masking(instWord >> 20) with 3 bits(1s)   so it will be instaword & 11111 then store the result in opcode
	funct7 = (instWord >> 25) & 0x0000007F;
	// — inst[31] — inst[30:25] inst[24:21] inst[20]
	I_imm = ((instWord >> 20) & 0x7FF) | (((instWord >> 31) ? 0xFFFFF800 : 0x0)); 
	/////////////////////																			  
																				
	S_imm = ((instWord >> 7) & 0x1F) | (((instWord >> 25) << 5) & 0x7E0) | (((instWord >> 31) ? 0xFFFFF800 : 0x0));
	//--------------
	//-------------
	U_imm = ((instWord >> 12) & 0xFFFFF) << 12;
	//--------------

	x = 0;
	y = 0;
	z = 0;
	x = ((instWord >> 12) & 0xFF) << 11;
	y = ((instWord >> 20) & 1) << 10;
	z = ((instWord >> 21) & 0x3FF);
	J_imm = x | y | z | ((instWord >> 31) ? 0xFFF80000 : 0x0);
	//----------------
	x = ((instWord >> 7) & 1) << 10;;
	y = ((instWord >> 8) & 0xF);
	z = ((instWord >> 25) & 0x3F) << 4;
	z = z | y;
	z = z | x;
	B_imm = (((instWord >> 31) ? 0xFFFFF800 : 0x0)) | z;
	//----
	printPrefix(pc, instWord);

	if (opcode == 51) {		// R Instructions

		{
			if (funct7 == 32 && funct3 == 0)
			{
				cout << "\tSUB\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n";
				regs[rd] = regs[rs1] - regs[rs2];
			}
			else if (funct7 == 0 && funct3 == 0)

			{
				cout << "\tADD\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n";
				regs[rd] = regs[rs1] + regs[rs2];

			}
			else if (funct7 == 0 && funct3 == 1)
			{
				cout << "\tSLL\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n";
				regs[rd] = regs[rs1] << regs[rs2];

			}
			else if (funct7 == 0 && funct3 == 2)
			{
				cout << "\tSLT\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n"; 
				regs[rd] = (regs[rs1] < regs[rs2]) ? 1 : 0;

			}
			else if (funct7 == 0 && funct3 == 3)
			{
				cout << "\tSLTU\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n";
				regs[rd] = (unsigned(regs[rs1]) < unsigned(regs[rs2])) ? 1 : 0;
			}
			else if (funct7 == 0 && funct3 == 4)
			{
				cout << "\tXOR\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n";
				regs[rd] = regs[rs1] ^ regs[rs2];
			}
			else if (funct7 == 0 && funct3 == 5)
			{
				cout << "\tSRL\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n";
				regs[rd] = (unsigned)(regs[rs1] >> regs[rs2]);
			}
			else if (funct7 == 32 && funct3 == 5)
			{
				cout << "\tSRA\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n";
				regs[rd] = (regs[rs1] >> (unsigned)regs[rs2]);
			}
			else if (funct7 == 0 && funct3 == 3)
			{
				cout << "\tSLTU\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n"; //SLTU
				regs[rd] = (unsigned(regs[rs1]) < unsigned(regs[rs2])) ? 1 : 0;
			}
			else if (funct7 == 0 && funct3 == 6)
			{
				cout << "\tOR\tx" << dec << rd << ", x" << dec << rs1 << ", x" << rs2 << "\n";
				regs[rd] = regs[rs1] | regs[rs2];
			}
			else if (funct7 == 0 && funct3 == 7)
			{
				cout << "\tAND\tx" << dec << rd << ", x" << dec << rs1 << ", x" << rs2 << "\n";
				regs[rd] = regs[rs1] & regs[rs2];
			}


			else
				cout << "\tUnkown R Instruction \n";
		}
	}

	else if (opcode == 19) {	// I instructions part 1

		switch (funct3) {
		case 0:
		{
			cout << "\tADDI\tx" << dec << rd << ", x" << dec << rs1 << ", " << dec << (int)I_imm << "\n";
			regs[rd] = regs[rs1] + (int)I_imm;

			break;
		}
		case 1: {
			cout << "\tSLLI\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << I_imm << "\n"; 
			unsigned int x = 0;
			x = (I_imm & 0x1F);
			regs[rd] = regs[rs1] << x;
			break;
		}

		case 2: {
			cout << "\tSLTI\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; 
			regs[rd] = (regs[rs1] < (int)I_imm) ? 1 : 0;
			break;
		}
		case 3: {
			cout << "\tSLTIU\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = ((unsigned)regs[rs1] < (unsigned)I_imm) ? 1 : 0;
			break;
		}
		case 4: {
			cout << "\tXORI\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = regs[rs1] ^ (int)I_imm;
			break;
		}
		case 5: {
			unsigned int x = 0;
			unsigned int temp = 0;
			x = I_imm & 0x1F;
			temp = (I_imm << 5) & 0x7F;
			if (temp == 0)
			{
				cout << "\tSRLI\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << I_imm << "\n";
				regs[rd] = regs[rs1] >> x;
			}
			else
			{
				cout << "\tSRAI\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << I_imm << "\n"; 
				regs[rd] = (regs[rs1] >> int(x));
			}
			break;
		}
		case 6: {
			cout << "\tORI\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = regs[rs1] | (int)I_imm;
			break;
		}
		case 7: {
			cout << "\tANDI\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = regs[rs1] & (int)I_imm;
			break;
		}
		default:
			cout << "\tUnkown I Instruction \n";
		}
	}
	else if (opcode == 3) {	// I instructions part 2

		switch (funct3) {
		case 0:
		{

			cout << "\tLB\tx" << rd << ", " << dec << (int)I_imm << "(" << "x" << rs1 << ")" << "\n"; 
			regs[rd] = memory[regs[rs1] + (int)I_imm];

			break;
		}
		case 1: {
			cout << "\tLH\tx" << rd << ", " << dec << (int)I_imm << "(" << "x" << rs1 << ")" << "\n";
			regs[rd] = memory[regs[rs1] + (int)I_imm] | (memory[regs[rs1] + (int)I_imm + 1] << 8);
			break;
		}
		case 2: {

			cout << "\tLW\tx" << rd << ", " << dec << (int)I_imm << "(x" << rs1 << ")" << "\n";
			regs[rd] = memory[regs[rs1] + (int)I_imm] | (memory[regs[rs1] + (int)I_imm + 1] << 8) | (memory[regs[rs1] + (int)I_imm + 2] << 16) | (memory[regs[rs1] + (int)I_imm + 3] << 24);
			break;
		}
		case 4: {
			cout << "\tLBU\tx" << ", " << dec << (unsigned)I_imm << "(" << "x" << rs1 << ")" << "\n";
			regs[rd] = (unsigned char)memory[regs[rs1] + (unsigned)I_imm];
			break;
		}
		case 6: {
			cout << "\tLHU\tx" << ", " << dec << (unsigned)I_imm << "(" << "x" << rs1 << ")" << "\n";
			regs[rd] = (unsigned char)memory[regs[rs1] + (unsigned)I_imm] | (memory[regs[rs1] + (unsigned)I_imm + 1] << 8);
			break;
		}


		default:
			cout << "\tUnkown I Instruction \n";
		}
	}
	else if (opcode == 35) // S intructions
	{
		switch (funct3) {
		case 0:
		{
			cout << "\tSB\tx" << rs2 << ", " << dec << (int)S_imm << "(" << "x" << rs1 << ")" << "\n"; 
			memory[regs[rs1] + (int)S_imm] = regs[rs2] & 0xff;
			break;
		}
		case 1: {
			cout << "\tSH\tx" << rs2 << ", " << dec << (int)S_imm << "(" << "x" << rs1 << ")" << "\n"; 
			memory[regs[rs1] + (int)S_imm] = regs[rs2] & 0xff;
			memory[regs[rs1] + (int)S_imm + 1] = (regs[rs2] >> 8) & 0xff;
			break;
		}
		case 2: {
			cout << "\tSW\tx" << rd << ", " << dec << (int)S_imm << "(x" << rs1 << ")" << "\n";
			memory[regs[rs1] + (int)S_imm] = regs[rs2] & 0xff;
			memory[regs[rs1] + (int)S_imm + 1] = (regs[rs2] >> 8) & 0xff;
			memory[regs[rs1] + (int)S_imm + 2] = (regs[rs2] >> 16) & 0xff;
			memory[regs[rs1] + (int)S_imm + 3] = (regs[rs2] >> 24) & 0xff;
			break;
		}
		default:
			cout << "\tUnkown S Instruction \n";
		}
	}
	else if (opcode == 99) // B instructions
	{

		switch (funct3)
		{
		case 0: {	//BEQ
			cout << "\tBEQ\tx" << dec << rs2 << ", " << "x" << dec << rs1 << ", 0x" << hex << pc + (int(B_imm)) * 2 << "\n";
			if (regs[rs1] == regs[rs2])
				pc = pc + (int(B_imm)) * 2 - 4;
			break;
		}
		case 1: {	//BNE
			cout << "\tbne\t " << "x" << rs1 << "," << "x" << rs2 << ", " << (int)B_imm << endl;
			if (regs[rs1] != regs[rs2]) {
				pc = pc + (int(B_imm)) * 2 - 4;
			}
		}
				break;
		case 4: {	//BLT
			cout << "\tBLT\tx" << dec << rs1 << ", " << "x" << dec << rs2 << ", 0x" << hex << pc + (int(B_imm)) * 2 << "\n";
			if (regs[rs1] < regs[rs2])
				pc = pc + (int(B_imm)) * 2 - 4;
			break;
		}

		case 5: {	//BGE
			cout << "\tBGE\tx" << dec << rs1 << ", " << "x" << dec << rs2 << ", 0x" << hex << pc + (int(B_imm)) * 2 << "\n";
			if (regs[rs1] >= regs[rs2])
				pc = pc + (int(B_imm)) * 2 - 4;
			break;
		}
		case 6: {	//BLTU
			cout << "\tBLTU\tx" << dec << rs1 << ", " << "x" << dec << rs2 << ", 0x" << hex << pc + (int(B_imm)) * 2 << "\n"; 

			if (unsigned(regs[rs1]) < unsigned(regs[rs2]))
				pc = pc + (int(B_imm)) * 2 - 4;
			break;
		}
		case 7: {	//BGEU
			cout << "\tBGEU\tx" << dec << rs1 << ", " << "x" << dec << rs2 << ", 0x" << hex << pc + (int(B_imm)) * 2 << "\n";
			if (unsigned(regs[rs1]) > unsigned(regs[rs2]))
				pc = pc + (int(B_imm)) * 2 - 4;
			break;
		}
		default: cout << "\tUnkown B Instruction \n";
		}
	}

	//U instructions
	else if (opcode == 55)
	{
		cout << "\tLUI\tx" << dec << rd << ", 0x" << hex << (int)U_imm << "\n";
		regs[rd] = regs[rd] | U_imm;
	}
	else if (opcode == 23)
	{
		cout << "\tAUIPC\tx" << dec << rd << ", " << hex << (int)U_imm << "\n";
		regs[rd] = instPC + (int)U_imm;
	}
	

	else if (opcode == 115) //ECALL
	{
		cout << "\tECALL\t" << endl;
		switch (regs[17])
		{
		case 1:
		{
			cout << dec << int(regs[10]) << endl;
			break;
		}
		case 4:
		{
			int c = 0;
			while (memory[regs[10]] != NULL)
			{
				cout << char(memory[regs[10]] + c);
				c++;
			}
			break;
		}
		case 5:
		{
			cin >> regs[10];
			break;
		}
		case 8:
		{
			int c = 0;
			for (int i = 0; i < regs[11]; i++)
			{
				cin.get(memory[regs[10] + c]); //re-check
				c++;
			}
			break;
		}
		case 10:
		{
			flag = false;
			break;
		}
		}

	}


	else if (opcode == 111) //JAL
	{
		cout << "\tJAL\tx" << dec << rd << ",0x" << hex << (int)J_imm << "\n";

		regs[rd] = pc;
		pc = (pc - 4) + int(J_imm) * 2;

	}
	else if (opcode == 103) //JALR
	{
		cout << "\tJALR\tx" << dec << rd << ", x" << dec << rs1 << ",0x" << hex << (int)I_imm << "\n";

		regs[rd] = pc;
		pc = (regs[rs1] + int(I_imm)) & 0xFFFFFFFE; //adds rs1 to immediate and sets rightmost bit 0
													//
													//regs[rd] = pc; //sets rd to pc+4 (pc already incremented)
													//instPC = (regs[rs1] + int(I_imm));// & 0xFFFFFFFE; //adds rs1 to immediate and sets rightmost bit 0

	}
	else {
		cout << "\tUnkown Instruction \n";
	}
}

int main(int argc, char* argv[]) {

	unsigned int instWord = 0;
	ifstream inFile;
	ofstream outFile;
	unsigned int OP32or62 = 0;
	if (argc < 1)
		emitError("use: rv32i_sim <machine_code_file_name>\n");

	inFile.open(argv[1], ios::in | ios::binary | ios::ate);

	if (inFile.is_open()) //if file opened
	{
		int fsize = inFile.tellg();
		bool flag = true;
		inFile.seekg(0, inFile.beg); // makes the pointer start from beginning of file   (0 bytes from beginning of file)
		if (!inFile.read((char*)memory, fsize))
			emitError("Cannot read from input file\n");

		while (flag) {
			instWord = (unsigned char)memory[pc] | (((unsigned char)memory[pc + 1]) << 8) | (((unsigned char)memory[pc + 2]) << 16) | (((unsigned char)memory[pc + 3]) << 24);
			pc += 4;
			instDecExec(instWord, flag);
		}
		// dump the registers
		for (int i = 0; i < 32; i++)
			cout << "x" << dec << i << ": \t" << "0x" << hex << std::setfill('0') << std::setw(8) << regs[i] << "\n";
	}
	else
		emitError("Cannot access input file\n");
	system("pause");
}