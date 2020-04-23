#pragma once

#include<stdlib.h>
#include<time.h>	//for random seed

#include"Memory.h"
#include"Input.h"
#include"defaults.h"
#include"Timer.h"

//opcode parser
#define HEAD (currentOpcode >> 12)
#define SUB (currentOpcode & 0x000f)
#define SUB_DUAL (currentOpcode & 0x00ff)
//?x??
#define VX (v[(currentOpcode & 0x0f00) >> 8])

//??y?
#define VY (v[(currentOpcode & 0x00f0) >> 4])

//vf
#define VF (v[0xf])

//??nn
#define NN (currentOpcode & 0x00ff)

//nnn
#define NNN (currentOpcode & 0x0fff)

#define N (currentOpcode & 0x000f)




#define STACK_SIZE 16
#define V_REGISTER_SIZE 0x10

class CPU;	//forward declaration for cputable
typedef void (CPU::*       CPUTable)();


class CPU final: public Input{
private:
	uint16_t currentOpcode;
	uint16_t programCounter;
	uint8_t stackPointer;
	uint16_t indexRegister;	//I register
	uint16_t stack[STACK_SIZE];
	uint8_t v[V_REGISTER_SIZE];


#define OPCODE_TABLE_SIZE 0x10
#define OPCODE_TABLE_0_SIZE 0x100
#define OPCODE_TABLE_8_SIZE 0x10
#define OPCODE_TABLE_E_SIZE 0x100
#define OPCODE_TABLE_F_SIZE 0x100
	//opcode table
	CPUTable opcode_table[OPCODE_TABLE_SIZE];
	CPUTable opcode_table_0[OPCODE_TABLE_0_SIZE];
	CPUTable opcode_table_8[OPCODE_TABLE_8_SIZE];
	CPUTable opcode_table_e[OPCODE_TABLE_E_SIZE];
	CPUTable opcode_table_f[OPCODE_TABLE_F_SIZE];

	//jumboLUT version of opcode table
#define JUMBO_TABLE_SIZE 0x10000
	CPUTable jumbo_table[JUMBO_TABLE_SIZE];
	
	uint16_t controllerOp = 0x0;
	//TODO
	bool throwError;

	/*	===replaced by macros===
	uint8_t head;
	uint8_t sub_dual;
	uint8_t sub;
	uint8_t *vx;
	uint8_t *vy;
	uint8_t *vf;
	uint8_t nn;
	uint16_t nnn;
	uint8_t n;
	*/

	//1 = is a jump; dont increment pc
	int flag;

	Memory* memory;
	uint8_t* delayRegister;
	uint8_t* pressedKey;

	bool jmpHint = false;

public:
	//inline getters
	uint16_t* getProgramCounter(){ return &programCounter; }
	uint8_t* getStackPointer(){ return &stackPointer; }
	uint16_t* getIndexRegister(){ return &indexRegister; }
	uint16_t* getStack(uint8_t input){ return &stack[input]; }
	uint8_t* getV(uint8_t input){ return &v[input]; }


	//return previous jmp state and reset
	bool prevJmpHint(){ 
		bool temp = jmpHint;
		jmpHint = false;	//reset
		return temp;
	}

	//interpreter needs memory to access, a 60hz delay register(not implemented in cpu), a fetched opcode, and input key
	uint16_t decode(); //current opcode decoder
	uint16_t fetch();

	uint16_t decode_jumboLUT(); //even more sparsely populated table

	//for switch method
	uint16_t decode(Memory* memory, uint8_t *delayRegister, uint16_t currentOpcode, uint8_t pressedKey); //current opcode decoder
	uint16_t fetch(Memory* memory);
	
	//opcodes
	void opcodetoTable0();
	void opcodetoTable8();
	void opcodetoTablee();
	void opcodetoTablef();
	void opcode00e0();
	void opcode00ee();
	void opcode0nnn();
	void opcode1nnn();
	void opcode2nnn();
	void opcode3xnn();
	void opcode4xnn();
	void opcode5xy0();
	void opcode6xnn();
	void opcode7xnn();
	void opcode8xy0();
	void opcode8xy1();
	void opcode8xy2();
	void opcode8xy3();
	void opcode8xy4();
	void opcode8xy5();
	void opcode8xy6();
	void opcode8xy7();
	void opcode8xye();
	void opcode9xy0();
	void opcodeannn();
	void opcodebnnn();
	void opcodecxnn();
	void opcodedxyn();
	void opcodeex9e();
	void opcodeexa1();
	void opcodefx07();
	void opcodefx0a();
	void opcodefx15();
	void opcodefx18();
	void opcodefx1e();
	void opcodefx29();
	void opcodefx33();
	void opcodefx55();
	void opcodefx65();
	void opcodenull();

	void init(Memory* memory, uint8_t* delayRegister, uint8_t* pressedKey);
	
};

//loop inline
inline uint16_t CPU::fetch(Memory* memory){
	uint16_t currentOpcode = 0;

	//fetch - mem is 8bit, opcode is 16bit, big endian
	//mem[pc] as top 8bit + mem[pc+1] as bottom 8bit = 16bit
	currentOpcode = memory->read(programCounter) << 8;
	currentOpcode |= memory->read(programCounter + 1);


	//this is fetch -> programCounter++ -> execute
	//programCounter += 2;


	return currentOpcode;
}


inline uint16_t CPU::fetch(){

	//fetch - mem is 8bit, opcode is 16bit, big endian
	//mem[pc] as top 8bit + mem[pc+1] as bottom 8bit = 16bit
	currentOpcode = memory->read(programCounter) << 8;
	currentOpcode |= memory->read(programCounter + 1);


	//program counter increments by 2. 16bit addr
	//nn -byte(2 nibbles)
	//nnn -addr(3 nibbles)
	//x, y -Vx, Vy registers
	//index register

	//1 = is a jump; dont increment pc
	flag = 0;

	//just in case some other parts of the machine needs opcode
	return currentOpcode;
}

inline uint16_t CPU::decode_jumboLUT(){
	throwError = false;
	controllerOp = 0x0; //this motherfucker

	//opcode table
	(this->*(jumbo_table[currentOpcode]))();

	if (throwError == true)
		fprintf(stderr, "unsupported opcode!: %x\n", currentOpcode);

	if (flag != 1)	//only if its not jump
		programCounter += 2; //increment after fetch	

	return controllerOp;

}

inline uint16_t CPU::decode(){
	throwError = false;
	controllerOp = 0x0; //this motherfucker

	//opcode table
	(this->*(opcode_table[HEAD]))();
	
	if (throwError == true)
		fprintf(stderr, "unsupported opcode!: %x\n", currentOpcode);

	if (flag != 1)	//only if its not jump
	programCounter += 2; //increment after fetch	

	return controllerOp;

}


//for switch method
inline uint16_t CPU::decode(Memory* memory, uint8_t* delayRegister, uint16_t currentOpcode, uint8_t pressedKey){
	uint16_t controllerOp = 0x0;

	//TODO
	bool throwError = false; //if illegal instruction

	//program counter increments by 2. 16bit addr
	//nn -byte(2 nibbles)
	//nnn -addr(3 nibbles)
	//x, y -Vx, Vy registers
	//index register

	//1 = is a jump; dont increment pc
	int flag = 0;

	//first nibble
	switch (currentOpcode >> 12){
	case 0x0:
		switch (SUB_DUAL){
		case 0xe0:	controllerOp = 0x1;
			break;
		case 0xee:	programCounter = stack[--stackPointer]; //return from SUBroutine	(and increment pc after to get out of loop)
			break;
		default:	stack[stackPointer++] = programCounter; programCounter = NNN; flag = 1;//call SUBroutine from nnn	(dont increment pc)
			break;
		}
		break;
	case 0x1:	programCounter = NNN; flag = 1;//jump to nnn	(dont increment pc)
				jmpHint = true; //hint for video flicker loop
		break;
	case 0x2:	stack[stackPointer++] = programCounter; programCounter = NNN; flag = 1;//call SUBroutine from nnn	(dont increment pc)
		break;
	case 0x3:	if (VX == NN) programCounter += 2; //skip if ==
		break;
	case 0x4:	if (VX != NN) programCounter += 2; //skip if !=
		break;
	case 0x5:	if (VX == VY) programCounter += 2; //skip if vx == vy
		break;
	case 0x6:	VX = NN; //into
		break;
	case 0x7:	VX += NN;
		break;
	case 0x8:
		switch (SUB){
		case 0x0:	VX = VY;
			break;
		case 0x1:	VX |= VY;
			break;
		case 0x2:	VX &= VY;
			break;
		case 0x3:	VX ^= VY;
			break;
		case 0x4:	VF = (VX + VY > 0xff) ? 0x1 : 0x0; VX += VY;
			break;
		case 0x5:	VF = (VX < VY) ? 0x0 : 0x1; VX -= VY;
			break;
		case 0x6:	VF = VX << 7; VF >>= 7; VX >>= 1;
			break;
		case 0x7:	VF = (VY < VX) ? 0x0 : 0x1; VX = VY - VX;
			break;
		case 0xe:	VF = VX >> 7; VX <<= 1;
			break;
		default: throwError = true;
			break;
		}
		break;
	case 0x9:	if (VX != VY) programCounter += 2; //skip if vx != vy
		break;
	case 0xa:	indexRegister = NNN;
		break;
	case 0xb:	programCounter = NNN + v[0]; flag = 1; //(dont increment pc)
		break;
	case 0xc:	VX = (rand() % 0x100) & NN;	//random
		break;
	case 0xd:	controllerOp = 0x2;
		break;
	case 0xe:
		switch (SUB_DUAL){
		case 0x9e:
			if (pressedKey == VX) programCounter += 2;
			break;
		case 0xa1:
			if (pressedKey != VX) programCounter += 2;
			break;
		default: throwError = true;
			break;
		}
		break;
	case 0xf:
		switch (SUB_DUAL){
		case 0x07:	VX = *delayRegister;
			break;
		case 0x0a:	if (Input::isKeyPressed(pressedKey) == true) VX = pressedKey; else flag = 1; //wait again	(dont increment pc)
			break;
		case 0x15:	*delayRegister = VX;
			break;
		case 0x18:	controllerOp = 0x3;
			break;
		case 0x1e:	indexRegister += VX;
			break;
		case 0x29:	indexRegister = VX * 5;	//font is stored at mem[0 ~ FONT_COUNT * 5]
			break;
		case 0x33:	//bcd code
			memory->write(indexRegister, VX / 100);
			memory->write(indexRegister + 1, (VX / 10) % 10);
			memory->write(indexRegister + 2, VX % 10);
			break;
		case 0x55:	for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++) memory->write(indexRegister + i, v[i]);
			break;
		case 0x65:	for (int i = 0; i <= (currentOpcode & 0x0f00) >> 8; i++) v[i] = memory->read(indexRegister + i);
			break;
		default: throwError = true;
			break;
		}
		break;
	default: throwError = true;
		break;
	}

	if (throwError == true)
		fprintf(stderr, "unsupported opcode!: %x\n", currentOpcode);

	if (flag != 1)	//only if its not jump
		programCounter += 2; //increment after fetch




	return controllerOp;

}


