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

public:
	//inline getters
	uint16_t* getProgramCounter(){ return &programCounter; }
	uint8_t* getStackPointer(){ return &stackPointer; }
	uint16_t* getIndexRegister(){ return &indexRegister; }
	uint16_t* getStack(uint8_t input){ return &stack[input]; }
	uint8_t* getV(uint8_t input){ return &v[input]; }


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

